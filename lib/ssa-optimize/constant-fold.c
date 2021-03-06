#include "scc/core/num.h"
#include "scc/ssa-optimize/optimize.h"
#include "scc/ssa/block.h"
#include "scc/ssa/context.h"
#include "scc/tree/target.h"
#include "scc/tree/type.h"

static op_result ssa_eval_cmp(int r1, int r2, struct num* l, struct num* r)
{
        int cr = num_cmp(l, r);
        init_int(l, cr == r1 || cr == r2, 32);
        return OR_OK;
}

static op_result ssa_eval_binop(ssa_binop_kind opcode, struct num* l, struct num* r)
{
        switch (opcode)
        {
                case SBIK_MUL: return num_mul(l, r);
                case SBIK_DIV: return num_div(l, r);
                case SBIK_MOD: return num_mod(l, r);
                case SBIK_ADD: return num_add(l, r);
                case SBIK_SUB: return num_sub(l, r);
                case SBIK_SHL: return num_bit_shl(l, r);
                case SBIK_SHR: return num_bit_shr(l, r);
                case SBIK_AND: return num_bit_and(l, r);
                case SBIK_OR: return num_bit_or(l, r);
                case SBIK_XOR: return num_bit_xor(l, r);
                case SBIK_LE: return ssa_eval_cmp(-1, -1, l, r);
                case SBIK_GR: return ssa_eval_cmp(1, 1, l, r);
                case SBIK_LEQ: return ssa_eval_cmp(-1, 0, l, r);
                case SBIK_GEQ: return ssa_eval_cmp(1, 0, l, r);
                case SBIK_EQ: return ssa_eval_cmp(0, 0, l, r);
                case SBIK_NEQ: return ssa_eval_cmp(-1, 1, l, r);

                default:
                        assert(0 && "Invalid binary instruction");
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

        struct num l = *ssa_get_constant_cvalue(lhs);
        struct num r = *ssa_get_constant_cvalue(rhs);
        ssa_eval_binop(ssa_get_binop_kind(instr), &l, &r);
        return ssa_new_constant(context,
                ssa_get_value_type(ssa_get_instr_cvar(instr)), &l);
}

static ssa_value* ssa_constant_fold_cast(ssa_context* context, ssa_instr* instr)
{
        ssa_value* operand = ssa_get_instr_operand_value(instr, 0);
        if (ssa_get_value_kind(operand) != SVK_CONSTANT)
                return NULL;

        struct num v = *ssa_get_constant_cvalue(operand);
        tree_type* to = ssa_get_value_type(ssa_get_instr_cvar(instr));
        const tree_target_info* target = ssa_get_target(context);

        if (tree_type_is_integer(to)) {
                unsigned num_bits = 8 * tree_get_sizeof(target, to);
                if (tree_type_is_signed_integer(to))
                        num_to_int(&v, num_bits);
                else
                        num_to_uint(&v, num_bits);
        }
        else if (tree_builtin_type_is(to, TBTK_FLOAT))
                num_to_f32(&v);
        else if (tree_builtin_type_is(to, TBTK_DOUBLE))
                num_to_f64(&v);
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

extern void ssa_fold_constants(const ssa_pass* pass)
{
        SSA_FOREACH_FUNCTION_BLOCK(pass->function, block)
        {
                ssa_instr* next = NULL;
                for (ssa_instr* instr = ssa_get_block_instrs_begin(block),
                        *end = ssa_get_block_instrs_end(block);
                        instr != end;
                        instr = next)
                {
                        next = ssa_get_next_instr(instr);
                        ssa_constant_fold_instr(pass->context, instr);
                }
        }
}
