#include "scc/ssa/ssa-opt.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-function.h"
#include "scc/ssa/ssa-branch.h"

static op_result ssa_eval_cmp(cmp_result r1, cmp_result r2, avalue* l, avalue* r)
{
        cmp_result cr = avalue_cmp(l, r);
        avalue_init_int(l, 32, true, cr == r1 || cr == r2 ? 1 : 0);
        return OR_OK;
}

static op_result ssa_eval_binop(ssa_binary_instr_kind opcode, avalue *l, avalue* r)
{
        SSA_ASSERT_BINARY_INSTR_KIND(opcode);
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

static ssa_value* ssa_opt_constant_fold_binop(ssa_context* context, ssa_instr* instr)
{
        ssa_value* lhs = ssa_get_binop_lhs(instr);
        ssa_value* rhs = ssa_get_binop_rhs(instr);

        if (ssa_get_value_kind(lhs) != SVK_CONSTANT
                || ssa_get_value_kind(rhs) != SVK_CONSTANT)
        {
                return NULL;
        }

        avalue l = ssa_get_constant_value(lhs);
        avalue r = ssa_get_constant_value(rhs);
        ssa_eval_binop(ssa_get_binop_kind(instr), &l, &r);

        return ssa_new_constant(context,
                ssa_get_value_type(ssa_get_instr_cvar(instr)), l);
}

static ssa_value* ssa_opt_constant_fold_cast(ssa_context* context, ssa_instr* instr)
{
        ssa_value* operand = ssa_get_cast_operand(instr);
        if (ssa_get_value_kind(operand) != SVK_CONSTANT)
                return NULL;

        avalue v = ssa_get_constant_value(operand);
        tree_type* t = ssa_get_value_type(ssa_get_instr_cvar(instr));
        const tree_target_info* target = ssa_get_target(context);

        if (tree_type_is_pointer(t))
        {
                if (avalue_is_zero(&v))
                        return ssa_new_null_pointer(context, t);

                avalue_to_int(&v, 8 * tree_get_pointer_size(target), false);
        }
        else if (tree_type_is_integer(t))
                avalue_to_int(&v, 8 * tree_get_sizeof(target, t),
                        tree_type_is_signed_integer(t));
        else if (tree_builtin_type_is(t, TBTK_FLOAT))
                avalue_to_sp(&v);
        else if (tree_builtin_type_is(t, TBTK_DOUBLE))
                avalue_to_dp(&v);

        return ssa_new_constant(context, t, v);
}

static void ssa_constant_fold_instr(ssa_context* context, ssa_instr* instr, htab* constants)
{
        ssa_value* constant = NULL;
        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_BINARY)
                constant = ssa_opt_constant_fold_binop(context, instr);
        else if (k == SIK_CAST)
                constant = ssa_opt_constant_fold_cast(context, instr);

        if (!constant)
                return;

        htab_insert_ptr(constants, ssa_get_value_id(ssa_get_instr_cvar(instr)), constant);
        ssa_remove_instr(instr);
}

static void ssa_constant_fold_instr_operands(ssa_instr* instr, htab* constants)
{
        for (ssa_value** it = ssa_get_instr_operands_begin(instr),
                **end = ssa_get_instr_operands_end(instr); it != end; it++)
        {
                // todo: ??
                // offset of getaddr maybe NULL
                if (!*it)
                        continue;

                hiter res;
                if (htab_find(constants, ssa_get_value_id(*it), &res))
                        *it = hiter_get_ptr(&res);
        }
}

static void ssa_constant_fold_branch_operands(ssa_branch* branch, htab* constants)
{
        ssa_branch_kind k = ssa_get_branch_kind(branch);
        hiter res;
        if (k == SBK_IF)
        {
                ssa_value* cond = ssa_get_if_cond(branch);
                if (htab_find(constants, ssa_get_value_id(cond), &res))
                        ssa_set_if_cond(branch, hiter_get_ptr(&res));
        }
        else if (k == SBK_RETURN)
        {
                ssa_value* val = ssa_get_return_value(branch);
                if (val && htab_find(constants, ssa_get_value_id(val), &res))
                        ssa_set_return_value(branch, hiter_get_ptr(&res));
        }
}

extern void ssa_fold_constants(ssa_context* context, ssa_function* function)
{
        htab constants;
        htab_init_ex_ptr(&constants, ssa_get_alloc(context));
        SSA_FOREACH_FUNCTION_BLOCK(function, block)
        {
                ssa_instr* next = NULL;
                for (ssa_instr* instr = ssa_get_block_instrs_begin(block),
                        *end = ssa_get_block_instrs_end(block);
                        instr != end;
                        instr = next)
                {
                        next = ssa_get_next_instr(instr);
                        ssa_constant_fold_instr_operands(instr, &constants);
                        ssa_constant_fold_instr(context, instr, &constants);
                }
                ssa_constant_fold_branch_operands(ssa_get_block_exit(block), &constants);
        }
        htab_dispose(&constants);
}

static void ssa_opt_eliminate_dead_if_branches(ssa_context* context, ssa_function* func)
{
        SSA_FOREACH_FUNCTION_BLOCK(func, block)
        {
                ssa_branch* exit = ssa_get_block_exit(block);
                if (ssa_get_branch_kind(exit) != SBK_IF)
                        continue;

                ssa_value* cond = ssa_get_if_cond(exit);
                if (ssa_get_value_kind(cond) != SVK_CONSTANT)
                        continue;

                avalue val = ssa_get_constant_value(cond);
                ssa_value* live_branch = avalue_is_zero(&val)
                        ? ssa_get_if_false_block(exit)
                        : ssa_get_if_true_block(exit);

                ssa_branch* jmp = ssa_new_jump_branch(context, live_branch);
                ssa_set_block_exit(block, jmp);
        }
}

static void ssa_opt_eliminate_dead_blocks(ssa_context* context, ssa_function* func)
{
        ssa_block* first = ssa_get_function_blocks_begin(func);
        ssa_block* end = ssa_get_function_blocks_end(func);
        if (first == end)
                return;

        htab visited;
        htab_init_ex_i8(&visited, ssa_get_alloc(context));

        dseq queue;
        dseq_init_ex_ptr(&queue, ssa_get_alloc(context));
        dseq_append_ptr(&queue, ssa_get_block_label(first));
        while (dseq_size(&queue))
        {
                ssa_value* label = dseq_pop_ptr(&queue);
                ssa_id id = ssa_get_value_id(label);
                if (htab_exists(&visited, id))
                        continue;

                htab_insert_i8(&visited, id, 1);
                ssa_branch* exit = ssa_get_block_exit(ssa_get_label_block(label));
                if (ssa_get_branch_kind(exit) == SBK_IF)
                {
                        dseq_append_ptr(&queue, ssa_get_if_true_block(exit));
                        dseq_append_ptr(&queue, ssa_get_if_false_block(exit));
                }
                else if (ssa_get_branch_kind(exit) == SBK_JUMP)
                        dseq_append_ptr(&queue, ssa_get_jump_dest(exit));
        }
        dseq_dispose(&queue);

        ssa_block* it = first;
        while (it != end)
        {
                ssa_block* next = ssa_get_next_block(it);
                ssa_id id = ssa_get_value_id(ssa_get_block_clabel(it));
                if (!htab_exists(&visited, id))
                        ssa_remove_block(it);
                it = next;
        }
        htab_dispose(&visited);
}


extern void ssa_eliminate_dead_code(ssa_context* context, ssa_function* function)
{
        ssa_opt_eliminate_dead_if_branches(context, function);
        ssa_opt_eliminate_dead_blocks(context, function);
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