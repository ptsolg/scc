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
        }
        S_UNREACHABLE();
        return OR_INVALID;
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
        ssa_eval_binop(ssa_get_binop_opcode(instr), &l, &r);

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
                avalue_to_int(&v, 8 * tree_get_pointer_size(target), false);
        else if (tree_type_is_integer(t))
                avalue_to_int(&v, 8 * tree_get_sizeof(target, t),
                        tree_type_is_signed_integer(t));
        else if (tree_builtin_type_is(t, TBTK_FLOAT))
                avalue_to_sp(&v);
        else if (tree_builtin_type_is(t, TBTK_DOUBLE))
                avalue_to_dp(&v);

        return ssa_new_constant(context, t, v);
}

static void ssa_opt_constant_fold_binop_operands(ssa_instr* instr, const htab* constants)
{
        ssa_value* lhs = ssa_get_binop_lhs(instr);
        ssa_value* rhs = ssa_get_binop_rhs(instr);

        hiter res;
        if (htab_find(constants, ssa_get_value_id(lhs), &res))
                ssa_set_binop_lhs(instr, hiter_get_ptr(&res));
        if (htab_find(constants, ssa_get_value_id(rhs), &res))
                ssa_set_binop_rhs(instr, hiter_get_ptr(&res));
}

static void ssa_opt_constant_fold_cast_operand(ssa_instr* instr, const htab* constants)
{
        ssa_value* operand = ssa_get_cast_operand(instr);

        hiter res;
        if (htab_find(constants, ssa_get_value_id(operand), &res))
                ssa_set_cast_operand(instr, hiter_get_ptr(&res));
}

static void ssa_opt_constant_fold_call_args(ssa_instr* instr, const htab* constants)
{
        hiter res;
        SSA_FOREACH_CALL_ARG(instr, it)
                if (htab_find(constants, ssa_get_value_id(*it), &res))
                        *it = hiter_get_ptr(&res);
}

static void ssa_opt_constant_fold_getaddr_operands(ssa_instr* instr, const htab* constants)
{
        ssa_value* index = ssa_get_getaddr_index(instr);
        ssa_value* offset = ssa_get_getaddr_offset(instr);

        hiter res;
        if (htab_find(constants, ssa_get_value_id(index), &res))
                ssa_set_getaddr_index(instr, hiter_get_ptr(&res));
        if (htab_find(constants, ssa_get_value_id(offset), &res))
                ssa_set_getaddr_offset(instr, hiter_get_ptr(&res));
}

static void ssa_opt_constant_fold_phi_args(ssa_instr* instr, const htab* constants)
{
        hiter res;
        SSA_FOREACH_PHI_ARG(instr, it)
                if (htab_find(constants, ssa_get_value_id(*it), &res))
                        *it = hiter_get_ptr(&res);
}

static void ssa_opt_constant_fold_load_operands(ssa_instr* instr, const htab* constants)
{
        ssa_value* what = ssa_get_load_what(instr);

        hiter res;
        if (htab_find(constants, ssa_get_value_id(what), &res))
                ssa_set_load_what(instr, hiter_get_ptr(&res));
}

static void ssa_opt_constant_fold_store_operands(ssa_instr* instr, const htab* constants)
{
        ssa_value* what = ssa_get_store_what(instr);

        hiter res;
        if (htab_find(constants, ssa_get_value_id(what), &res))
                ssa_set_store_what(instr, hiter_get_ptr(&res));
}

static void ssa_opt_constant_fold_instr_operands(ssa_instr* instr, const htab* constants)
{
        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_BINARY)
                ssa_opt_constant_fold_binop_operands(instr, constants);
        else if (k == SIK_CAST)
                ssa_opt_constant_fold_cast_operand(instr, constants);
        else if (k == SIK_CALL)
                ssa_opt_constant_fold_call_args(instr, constants);
        else if (k == SIK_GETADDR)
                ssa_opt_constant_fold_getaddr_operands(instr, constants);
        else if (k == SIK_PHI)
                ssa_opt_constant_fold_phi_args(instr, constants);
        else if (k == SIK_LOAD)
                ssa_opt_constant_fold_load_operands(instr, constants);
        else if (k == SIK_STORE)
                ssa_opt_constant_fold_store_operands(instr, constants);
}

static void ssa_opt_constant_fold_instr(
        ssa_context* context, ssa_instr* instr, htab* constants)
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

static void ssa_opt_constant_fold_branch_operands(ssa_branch* br, htab* constants)
{
        ssa_branch_kind k = ssa_get_branch_kind(br);
        hiter res;
        if (k == SBK_IF)
        {
                ssa_value* cond = ssa_get_if_cond(br);
                if (htab_find(constants, ssa_get_value_id(cond), &res))
                        ssa_set_if_cond(br, hiter_get_ptr(&res));
        }
        else if (k == SBK_RETURN)
        {
                ssa_value* val = ssa_get_return_value(br);
                if (val && htab_find(constants, ssa_get_value_id(val), &res))
                        ssa_set_return_value(br, hiter_get_ptr(&res));
        }
}

extern void ssa_opt_constant_fold(ssa_context* context, ssa_module* module)
{
        htab constants;
        htab_init_ex_ptr(&constants, ssa_get_alloc(context));

        SSA_FOREACH_MODULE_DEF(module, func)
        {
                SSA_FOREACH_FUNCTION_BLOCK(func, block)
                {
                        ssa_instr* next = NULL;
                        for (ssa_instr* instr = ssa_get_block_instrs_begin(block),
                                *end = ssa_get_block_instrs_end(block);
                                instr != end;
                                instr = next)
                        {
                                next = ssa_get_next_instr(instr);
                                ssa_opt_constant_fold_instr_operands(instr, &constants);
                                ssa_opt_constant_fold_instr(context, instr, &constants);
                        }
                        ssa_opt_constant_fold_branch_operands(
                                ssa_get_block_exit(block), &constants);
                }
        }

        htab_dispose(&constants);
}

extern void ssa_opt_init_constant_fold_pass(ssa_pass* pass)
{
        ssa_pass_init(pass, &ssa_opt_constant_fold);
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

extern void ssa_opt_eliminate_dead_code(ssa_context* context, ssa_module* module)
{
        SSA_FOREACH_MODULE_DEF(module, func)
        {
                ssa_opt_eliminate_dead_if_branches(context, func);
                ssa_opt_eliminate_dead_blocks(context, func);
        }
}

extern void ssa_opt_init_eliminate_dead_code_pass(ssa_pass* pass)
{
        ssa_pass_init(pass, &ssa_opt_eliminate_dead_code);
}

extern void ssa_init_optimizer(ssa_optimizer* self)
{
        for (int i = 0; i < SOK_SIZE; i++)
                self->enabled[i] = false;

        ssa_opt_init_constant_fold_pass(self->passes + SOK_CONSTANT_FOLDING);
        ssa_opt_init_eliminate_dead_code_pass(self->passes + SOK_DEAD_CODE_ELIMINATION);
}

extern void ssa_optimizer_enable_pass(ssa_optimizer* self, ssa_opt_kind kind)
{
        self->enabled[kind] = true;
}

extern void ssa_optimizer_run(ssa_optimizer* self, ssa_context* context, ssa_module* module)
{
        ssa_pass_manager manager;
        ssa_pass_manager_init(&manager);
        for (int i = 0; i < SOK_SIZE; i++)
                if (self->enabled[i])
                        ssa_pass_manager_add_pass(&manager, self->passes + i);

        ssa_pass_manager_run(&manager, context, module);

        SSA_FOREACH_MODULE_DEF(module, func)
                ssa_fix_function_content_uids(func);
}