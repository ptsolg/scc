#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-function.h"
#include "scc/tree/tree-expr.h"

static ssa_value* ssaize_mul(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_mul(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_div(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_div(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_mod(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_mod(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_add(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_add(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_sub(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_sub(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_shl(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_shl(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_shr(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_shr(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_le(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_le(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_gr(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_gr(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_leq(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_leq(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_geq(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_geq(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_eq(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_eq(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_neq(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_neq(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_and(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_and(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_xor(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_xor(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_or(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_or(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        if (!ssa_build_store(&self->builder, rhs, lhs))
                return NULL;

        return rhs;
}

static ssa_value*(*ssaize_binary_expr_table[TBK_SIZE])(ssaizer*, ssa_value*, ssa_value*);
static ssa_value* ssaize_dereference(ssaizer*, ssa_value*);
static inline ssa_value* _ssaize_binary_expr(ssaizer*, tree_binop_kind, ssa_value*, ssa_value*);

static inline ssa_value* _ssaize_assign(ssaizer* self,
        tree_binop_kind prefix, ssa_value* lhs, ssa_value* rhs)
{
        ssa_value* val = ssaize_dereference(self, lhs);
        if (!val)
                return NULL;

        TREE_ASSERT_BINOP_KIND(prefix);
        ssa_value* action = _ssaize_binary_expr(self, prefix, val, rhs);
        if (!action)
                return NULL;

        return ssaize_assign(self, lhs, action);
}

static ssa_value* ssaize_add_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_ADD, lhs, rhs);
}

static ssa_value* ssaize_sub_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_SUB, lhs, rhs);
}

static ssa_value* ssaize_mul_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_MUL, lhs, rhs);
}

static ssa_value* ssaize_div_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_DIV, lhs, rhs);
}

static ssa_value* ssaize_mod_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_MOD, lhs, rhs);
}

static ssa_value* ssaize_shl_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_SHL, lhs, rhs);
}

static ssa_value* ssaize_shr_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_SHR, lhs, rhs);
}

static ssa_value* ssaize_and_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_AND, lhs, rhs);
}

static ssa_value* ssaize_xor_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_XOR, lhs, rhs);
}

static ssa_value* ssaize_or_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return _ssaize_assign(self, TBK_OR, lhs, rhs);
}

static ssa_value* ssaize_comma(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return rhs;
}

static ssa_value* ssaizer_try_constant_folding(
        ssaizer* self, tree_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value*(*ssaize_binary_expr_table[TBK_SIZE])(ssaizer*, ssa_value*, ssa_value*) =
{
        NULL,
        &ssaize_mul,
        &ssaize_div,
        &ssaize_mod,
        &ssaize_add,
        &ssaize_sub,
        &ssaize_shl,
        &ssaize_shr,
        &ssaize_le,
        &ssaize_gr,
        &ssaize_leq,
        &ssaize_geq,
        &ssaize_eq,
        &ssaize_neq,
        &ssaize_and,
        &ssaize_xor,
        &ssaize_or,
        NULL,
        NULL,
        &ssaize_assign,
        &ssaize_add_assign,
        &ssaize_sub_assign,
        &ssaize_mul_assign,
        &ssaize_div_assign,
        &ssaize_mod_assign,
        &ssaize_shl_assign,
        &ssaize_shr_assign,
        &ssaize_and_assign,
        &ssaize_xor_assign,
        &ssaize_or_assign,
        &ssaize_comma,
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssaize_binary_expr_table) == TBK_SIZE,
        "ssaize_binary_expr_table needs an update");

static inline ssa_value* _ssaize_binary_expr(
        ssaizer* self, tree_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        TREE_ASSERT_BINOP_KIND(opcode);
        ssa_value* res = ssaizer_try_constant_folding(self, opcode, lhs, rhs);
        return res ? res : ssaize_binary_expr_table[opcode](self, lhs, rhs);
}

typedef struct
{
        ssa_block* exit;
        ssa_instr* phi;
        ssa_builder* builder;
        tree_type* phi_type;
} ssa_br_expr_info;

static void ssa_init_br_expr_info(
        ssa_br_expr_info* self, ssa_builder* builder, tree_type* phi_type)
{
        self->exit = NULL;
        self->phi = NULL;
        self->phi_type = phi_type;
        self->builder = builder;
}

static ssa_block* ssa_br_expr_info_get_exit(ssa_br_expr_info* self)
{
        if (!self->exit)
                self->exit = ssa_new_block(
                        ssa_get_builder_context(self->builder),
                        ssa_builder_gen_uid(self->builder),
                        NULL);

        return self->exit;
}

static void ssa_br_expr_info_add_phi_var(ssa_br_expr_info* self, ssa_value* var)
{
        if (!self->phi)
        {
                ssa_block* block = ssa_builder_get_block(self->builder);
                ssa_builder_set_block(self->builder, ssa_br_expr_info_get_exit(self));
                self->phi = ssa_get_var_instr(ssa_build_phi(self->builder, self->phi_type));
                ssa_builder_set_block(self->builder, block);
        }

        ssa_add_phi_var(self->phi, var);
}

static inline ssa_value* ssaize_log_expr_lhs(
        ssaizer* self, ssa_br_expr_info* info, const tree_expr* expr)
{
        if (!tree_expr_is(expr, TEK_BINARY))
                return ssaize_expr_as_condition(self, expr);

        tree_binop_kind kind = tree_get_binop_kind(expr);
        S_ASSERT(kind == TBK_LOG_AND || kind == TBK_LOG_OR);

        ssa_value* lhs_cond = ssaize_log_expr_lhs(self, info, tree_get_binop_lhs(expr));
        if (!lhs_cond)
                return NULL;

        ssa_block* current = ssaizer_new_block(self);
        ssa_block* exit = ssa_br_expr_info_get_exit(info);

        // finish prev block
        ssa_block* on_true = kind == TBK_LOG_OR ? exit : current;
        ssa_block* on_false = kind == TBK_LOG_OR ? current : exit;
        if (!ssaizer_build_if(self, lhs_cond, on_true, on_false))
                return false;

        ssa_br_expr_info_add_phi_var(info, lhs_cond);
        ssaizer_enter_block(self, current);
        return ssaize_expr_as_condition(self, tree_get_binop_rhs(expr));;
}

static ssa_value* ssaize_log_expr(ssaizer* self, const tree_expr* expr)
{
        ssa_br_expr_info info;
        ssa_init_br_expr_info(&info, &self->builder, tree_get_expr_type(expr));

        ssa_value* last_cond = ssaize_log_expr_lhs(self, &info, expr);
        if (!last_cond)
                return NULL;

        ssa_block* exit = ssa_br_expr_info_get_exit(&info);
        if (!ssaizer_build_jmp(self, exit))
                return false;

        ssa_br_expr_info_add_phi_var(&info, last_cond);
        ssaizer_enter_block(self, exit);
        ssaizer_finish_block(self, exit);
        return ssa_get_instr_var(info.phi);
}

extern ssa_value* ssaize_binary_expr(ssaizer* self, const tree_expr* expr)
{
        tree_binop_kind k = tree_get_binop_kind(expr);
        if (k == TBK_LOG_AND || k == TBK_LOG_OR)
                return ssaize_log_expr(self, expr);

        ssa_value* lhs = ssaize_expr(self, tree_get_binop_lhs(expr));
        if (!lhs)
                return NULL;

        ssa_value* rhs = ssaize_expr(self, tree_get_binop_rhs(expr));
        if (!rhs)
                return NULL;

        return _ssaize_binary_expr(self, k, lhs, rhs);
}

static ssa_value* ssaize_pre_inc(ssaizer* self, ssa_value* operand)
{
        ssa_value* val = ssaize_dereference(self, operand);
        if (!val)
                return NULL;

        ssa_value* inc = ssa_build_inc(&self->builder, val);
        if (!inc || !ssaize_assign(self, operand, inc))
                return NULL;

        return inc;
}

static ssa_value* ssaize_pre_dec(ssaizer* self, ssa_value* operand)
{
        ssa_value* val = ssaize_dereference(self, operand);
        if (!val)
                return NULL;

        ssa_value* dec = ssa_build_dec(&self->builder, val);
        if (!dec || !ssaize_assign(self, operand, dec))
                return NULL;

        return dec;
}

static ssa_value* ssaize_post_inc(ssaizer* self, ssa_value* operand)
{
        ssa_value* val = ssaize_dereference(self, operand);
        if (!val)
                return NULL;

        ssa_value* inc = ssa_build_inc(&self->builder, val);
        if (!inc || !ssaize_assign(self, operand, inc))
                return NULL;

        return val;
}

static ssa_value* ssaize_post_dec(ssaizer* self, ssa_value* operand)
{
        ssa_value* val = ssaize_dereference(self, operand);
        if (!val)
                return NULL;

        ssa_value* dec = ssa_build_dec(&self->builder, val);
        if (!dec || !ssaize_assign(self, operand, dec))
                return NULL;

        return val;
}

static ssa_value* ssaize_plus(ssaizer* self, ssa_value* operand)
{
        return operand;
}

static ssa_value* ssaize_minus(ssaizer* self, ssa_value* operand)
{
        return ssa_build_neg(&self->builder, operand);
}

static ssa_value* ssaize_not(ssaizer* self, ssa_value* operand)
{
        return ssa_build_not(&self->builder, operand);
}

static ssa_value* ssaize_log_not(ssaizer* self, ssa_value* operand)
{
        return ssa_build_log_not(&self->builder, operand);
}

static ssa_value* ssaize_dereference(ssaizer* self, ssa_value* operand)
{
        return ssa_build_load(&self->builder, operand);
}

static ssa_value* ssaize_address(ssaizer* self, ssa_value* operand)
{
        return operand;
}

static ssa_value*(*ssaize_unary_expr_table[TUK_SIZE])(ssaizer*, ssa_value*) =
{
        NULL,
        &ssaize_post_inc,
        &ssaize_post_dec,
        &ssaize_pre_inc,
        &ssaize_pre_dec,
        &ssaize_plus,
        &ssaize_minus,
        &ssaize_not,
        &ssaize_log_not,
        &ssaize_dereference,
        &ssaize_address,
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssaize_unary_expr_table) == TUK_SIZE,
        "ssaize_unary_expr_table needs an update");

extern ssa_value* ssaize_unary_expr(ssaizer* self, const tree_expr* expr)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        TREE_ASSERT_UNOP_KIND(k);

        ssa_value* operand = ssaize_expr(self, tree_get_unop_expr(expr));
        if (!operand)
                return NULL;

        return ssaize_unary_expr_table[k](self, operand);
}

extern ssa_value* ssaize_call_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_subscript_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

static bool ssaize_conditional_expr_branch(
        ssaizer* self, ssa_br_expr_info* info, const tree_expr* branch)
{
        ssa_value* val = ssaize_expr(self, branch);
        if (!val)
                return false;

        ssa_br_expr_info_add_phi_var(info, val);
        return ssaizer_build_jmp(self, ssa_br_expr_info_get_exit(info));
}

extern ssa_value* ssaize_conditional_expr(ssaizer* self, const tree_expr* expr)
{
        ssa_value* cond = ssaize_expr_as_condition(self, tree_get_conditional_condition(expr));
        if (!cond)
                return NULL;
        
        ssa_br_expr_info info;
        ssa_init_br_expr_info(&info, &self->builder,
                tree_get_expr_type(tree_get_conditional_lhs(expr)));

        ssa_block* lblock = ssaizer_new_block(self);
        ssa_block* rblock = ssaizer_new_block(self);
        if (!ssaizer_build_if(self, cond, lblock, rblock))
                return false;

        ssaizer_enter_block(self, lblock);
        if (!ssaize_conditional_expr_branch(self, &info, tree_get_conditional_lhs(expr)))
                return NULL;

        ssaizer_enter_block(self, rblock);
        if (!ssaize_conditional_expr_branch(self, &info, tree_get_conditional_rhs(expr)))
                return NULL;

        ssaizer_enter_block(self, info.exit);
        return ssa_get_instr_var(info.phi);
}

extern ssa_value* ssaize_integer_literal(ssaizer* self, const tree_expr* expr)
{
        return ssa_build_int_constant(&self->builder,
                tree_get_expr_type(expr), tree_get_integer_literal(expr));
}

extern ssa_value* ssaize_character_literal(ssaizer* self, const tree_expr* expr)
{
        return ssa_build_int_constant(&self->builder,
                tree_get_expr_type(expr), tree_get_character_literal(expr));
}

extern ssa_value* ssaize_floating_literal(ssaizer* self, const tree_expr* expr)
{
        return ssa_build_sp_constant(&self->builder,
                tree_get_expr_type(expr), tree_get_floating_literal(expr));
}

extern ssa_value* ssaize_string_literal(ssaizer* self, const tree_expr* expr)
{
        return ssa_build_dp_constant(&self->builder,
                tree_get_expr_type(expr), (double)tree_get_floating_lliteral(expr));
}

extern ssa_value* ssaize_decl_expr(ssaizer* self, const tree_expr* expr)
{
        tree_decl* var = tree_get_decl_expr_entity(expr);
        if (!var)
                return NULL;

        ssa_value* def = ssaizer_get_def(self, var);
        S_ASSERT(def);
        return tree_expr_is_lvalue(expr)
                ? def
                : ssaize_dereference(self, def);
}

extern ssa_value* ssaize_member_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_cast_expr(ssaizer* self, const tree_expr* expr)
{
        ssa_value* operand = ssaize_expr(self, tree_get_cast_expr(expr));
        if (!operand)
                return NULL;

        return ssa_build_cast(&self->builder, tree_get_expr_type(expr), operand);
}

extern ssa_value* ssaize_sizeof_expr(ssaizer* self, const tree_expr* expr)
{
        tree_type* type = tree_sizeof_is_unary(expr)
                ? tree_get_expr_type(tree_get_sizeof_expr(expr))
                : tree_get_sizeof_type(expr);

        ssize size = tree_get_sizeof(ssa_get_target(self->context), type);
        return ssa_build_int_constant(&self->builder, tree_get_expr_type(expr), size);
}

extern ssa_value* ssaize_paren_expr(ssaizer* self, const tree_expr* expr)
{
        return ssaize_expr(self, tree_get_paren_expr(expr));
}

extern ssa_value* ssaize_init_expr(ssaizer* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssaize_impl_init_expr(ssaizer* self, const tree_expr* expr)
{
        return ssaize_expr(self, tree_get_impl_init_expr(expr));
}

static ssa_value* (*ssaize_expr_table[TEK_SIZE])(ssaizer*, const tree_expr*) =
{
        NULL, // TEK_UNKNOWN
        &ssaize_binary_expr, // TEK_BINARY
        &ssaize_unary_expr, // TEK_UNARY
        &ssaize_call_expr, // TEK_CALL
        &ssaize_subscript_expr, // TEK_SUBSCRIPT
        &ssaize_conditional_expr, // TEK_CONDITIONAL
        &ssaize_integer_literal, // TEK_INTEGER_LITERAL
        &ssaize_character_literal, // TEK_CHARACTER_LITERAL
        &ssaize_floating_literal, // TEK_FLOATING_LITERAL
        &ssaize_string_literal, // TEK_STRING_LITERAL
        &ssaize_decl_expr, // TEK_DECL
        &ssaize_member_expr, // TEK_MEMBER
        &ssaize_cast_expr, // TEK_EXPLICIT_CAST
        &ssaize_cast_expr, // TEK_IMPLICIT_CAST
        &ssaize_sizeof_expr, // TEK_SIZEOF
        &ssaize_paren_expr, // TEK_PAREN
        &ssaize_init_expr, // TEK_INIT
        &ssaize_impl_init_expr, // TEK_IMPL_INIT
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssaize_expr_table) == TEK_SIZE,
        "ssaize_expr_table needs an update");

extern ssa_value* ssaize_expr(ssaizer* self, const tree_expr* expr)
{
        S_ASSERT(expr);
        tree_expr_kind k = tree_get_expr_kind(expr);
        TREE_ASSERT_EXPR_KIND(k);
        return ssaize_expr_table[k](self, expr);
}

extern ssa_value* ssaize_expr_as_condition(ssaizer* self, const tree_expr* cond)
{
        ssa_value* v = ssaize_expr(self, cond);
        if (!v)
                return NULL;

        if (ssa_get_value_kind(v) != SVK_VARIABLE)
                return v;

        ssa_instr* i = ssa_get_var_instr(v);
        if (ssa_get_instr_kind(i) != SIK_BINARY)
                return ssa_build_neq_zero(&self->builder, v);

        switch (ssa_get_binop_opcode(i))
        {
                case SBIK_LE:
                case SBIK_GR:
                case SBIK_LEQ:
                case SBIK_GEQ:
                case SBIK_EQ:
                case SBIK_NEQ:
                        return v;

                default:
                        return ssa_build_neq_zero(&self->builder, v);
        }
}