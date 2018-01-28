#include "scc/ssa/ssa-opt.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-function.h"

static op_result ssa_eval_cmp(cmp_result r1, cmp_result r2, avalue* l, avalue* r)
{
        cmp_result cr = avalue_cmp(l, r);
        avalue_init_int(l, 32, true, cr == r1 || cr == r2 ? 1 : 0);
        return OR_OK;
}

static op_result ssa_eval_binop(ssa_binop_kind opcode, avalue *l, avalue* r)
{
        switch (opcode)
        {
                case SBIK_MUL: return avalue_mul(l, r);
                case SBIK_DIV: return avalue_div(l, r);
                case SBIK_MOD: return avalue_mod(l, r);
                case SBIK_ADD: return avalue_add(l, r);
                case SBIK_SUB: return avalue_sub(l, r);
                case SBIK_SHL: return avalue_shl(l, r);
                case SBIK_SHR: return avalue_shr(l, r);
                case SBIK_AND: return avalue_and(l, r);
                case SBIK_OR: return avalue_or(l, r);
                case SBIK_XOR: return avalue_xor(l, r);
                case SBIK_LE: return ssa_eval_cmp(CR_LE, CR_LE, l, r);
                case SBIK_GR: return ssa_eval_cmp(CR_GR, CR_GR, l, r);
                case SBIK_LEQ: return ssa_eval_cmp(CR_LE, CR_EQ, l, r);
                case SBIK_GEQ: return ssa_eval_cmp(CR_GR, CR_EQ, l, r);
                case SBIK_EQ: return ssa_eval_cmp(CR_EQ, CR_EQ, l, r);
                case SBIK_NEQ: return ssa_eval_cmp(CR_LE, CR_GR, l, r);

                default:
                        S_ASSERT(0 && "Invalid binary instruction");
                        return OR_INVALID;
        }
}

static ssa_value* ssa_constant_fold_binop(ssa_context* context, ssa_instr* instr)
{
        ssa_value* lhs = ssa_get_instr_operand_value(instr, 0);
        ssa_value* rhs = ssa_get_instr_operand_value(instr, 1);

        if (ssa_get_value_kind(lhs) != SVK_CONSTANT
                || ssa_get_value_kind(rhs) != SVK_CONSTANT)
        {
                return NULL;
        }

        avalue l = *ssa_get_constant_cvalue(lhs);
        avalue r = *ssa_get_constant_cvalue(rhs);
        ssa_eval_binop(ssa_get_binop_kind(instr), &l, &r);

        return ssa_new_constant(context,
                ssa_get_value_type(ssa_get_instr_cvar(instr)), &l);
}

static ssa_value* ssa_constant_fold_cast(ssa_context* context, ssa_instr* instr)
{
        ssa_value* operand = ssa_get_instr_operand_value(instr, 0);
        if (ssa_get_value_kind(operand) != SVK_CONSTANT)
                return NULL;

        avalue v = *ssa_get_constant_cvalue(operand);
        tree_type* to = ssa_get_value_type(ssa_get_instr_cvar(instr));
        const tree_target_info* target = ssa_get_target(context);

        if (tree_type_is_integer(to))
                avalue_to_int(&v, 8 * tree_get_sizeof(target, to),
                        tree_type_is_signed_integer(to));
        else if (tree_builtin_type_is(to, TBTK_FLOAT))
                avalue_to_sp(&v);
        else if (tree_builtin_type_is(to, TBTK_DOUBLE))
                avalue_to_dp(&v);
        else
                return NULL; // probably cast to pointer

        return ssa_new_constant(context, to, &v);
}

static void ssa_constant_fold_instr(ssa_context* context, ssa_instr* instr)
{
        ssa_value* constant = NULL;
        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_BINARY)
                constant = ssa_constant_fold_binop(context, instr);
        else if (k == SIK_CAST)
                constant = ssa_constant_fold_cast(context, instr);

        if (!constant)
                return;

        ssa_replace_value_with(ssa_get_instr_var(instr), constant);
        ssa_remove_instr(instr);
}

extern void ssa_fold_constants(ssa_context* context, ssa_function* function)
{
        SSA_FOREACH_FUNCTION_BLOCK(function, block)
        {
                ssa_instr* next = NULL;
                for (ssa_instr* instr = ssa_get_block_instrs_begin(block),
                        *end = ssa_get_block_instrs_end(block);
                        instr != end;
                        instr = next)
                {
                        next = ssa_get_next_instr(instr);
                        ssa_constant_fold_instr(context, instr);
                }
        }
}

static void ssa_constant_fold_conditional_jumps(ssa_context* context, ssa_function* func)
{
        SSA_FOREACH_FUNCTION_BLOCK(func, block)
        {
                ssa_instr* cond_jump = ssa_get_block_terminator(block);
                if (!cond_jump
                        || ssa_value_is_used(ssa_get_block_label(block))
                        || ssa_get_terminator_instr_kind(cond_jump) != STIK_CONDITIONAL_JUMP)
                {
                        continue;
                }

                ssa_value* cond = ssa_get_instr_operand_value(cond_jump, 0);
                if (ssa_get_value_kind(cond) != SVK_CONSTANT)
                        continue;

                bool cond_val = avalue_is_zero(ssa_get_constant_cvalue(cond));
                ssa_value* live_branch = ssa_get_instr_operand_value(cond_jump, cond_val ? 2 : 1);
                ssa_instr* inderect_jump = ssa_new_inderect_jump(context, live_branch);

                ssa_add_instr_after(inderect_jump, cond_jump);
                ssa_remove_instr(cond_jump);
        }
}

static void ssa_remove_unused_blocks(ssa_function* func)
{
        ssa_block* it = ssa_get_function_blocks_begin(func);
        ssa_block* end = ssa_get_function_blocks_end(func);
        if (it == end)
                return;

        it = ssa_get_next_block(it); // skip entry
        while (it != end)
        {
                ssa_block* next = ssa_get_next_block(it);
                if (!ssa_value_is_used(ssa_get_block_label(it)))
                        ssa_remove_block(it);
                it = next;
        }
}

extern void ssa_eliminate_dead_code(ssa_context* context, ssa_function* function)
{
        ssa_constant_fold_conditional_jumps(context, function);
        ssa_remove_unused_blocks(function);
}

static void ssa_run_constant_fold_pass(ssa_pass* pass, ssa_function* function)
{
        ssa_constant_fold_pass* self = (ssa_constant_fold_pass*)(
                (char*)pass - offsetof(ssa_constant_fold_pass, pass));
        ssa_fold_constants(self->context, function);
}

extern void ssa_init_constant_fold_pass(ssa_constant_fold_pass* self, ssa_context* context)
{
        ssa_init_pass(&self->pass, SPK_FUNCTION, &ssa_run_constant_fold_pass);
        self->context = context;
}

static void ssa_run_dead_code_elimination_pass(ssa_pass* pass, ssa_function* function)
{
        ssa_dead_code_elimination_pass* self = (ssa_dead_code_elimination_pass*)(
                (char*)pass - offsetof(ssa_dead_code_elimination_pass, pass));
        ssa_eliminate_dead_code(self->context, function);
}

extern void ssa_init_dead_code_elimination_pass(
        ssa_dead_code_elimination_pass* self, ssa_context* context)
{
        ssa_init_pass(&self->pass, SPK_FUNCTION, &ssa_run_dead_code_elimination_pass);
        self->context = context;
}

extern void ssa_reset_optimizer_opts(ssa_optimizer_opts* self)
{
        self->fold_constants = false;
        self->eliminate_dead_code = false;
}

extern void ssa_optimize(ssa_context* context,
        ssa_module* module, const ssa_optimizer_opts* opts)
{
        ssa_constant_fold_pass cf;
        ssa_init_constant_fold_pass(&cf, context);

        ssa_dead_code_elimination_pass dce;
        ssa_init_dead_code_elimination_pass(&dce, context);

        ssa_pass_manager pm;
        ssa_init_pass_manager(&pm);
        if (opts->fold_constants)
                ssa_pass_manager_add_pass(&pm, &cf.pass);
        if (opts->eliminate_dead_code)
                ssa_pass_manager_add_pass(&pm, &dce.pass);

        ssa_pass_manager_run(&pm, module);

        SSA_FOREACH_MODULE_DEF(module, func)
                ssa_fix_function_content_uids(func);
}