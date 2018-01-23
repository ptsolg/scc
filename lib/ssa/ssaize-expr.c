#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-function.h"
#include "scc/ssa/ssa-module.h"
#include "scc/tree/tree-expr.h"

static ssa_value* ssaize_add(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        return tree_type_is_pointer(lt) || tree_type_is_pointer(rt)
                ? ssa_build_ptradd(&self->builder, lhs, rhs)
                : ssa_build_add(&self->builder, lhs, rhs);
}

static ssa_value* ssaize_sub(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        ssa_value* neg;
        if (tree_type_is_pointer(lt))
        {
                if (!(neg = ssa_build_neg(&self->builder, rhs)))
                        return NULL;
                return ssa_build_ptradd(&self->builder, lhs, neg);
        }
        else if (tree_type_is_pointer(rt))
        {
                if (!(neg = ssa_build_neg(&self->builder, lhs)))
                        return NULL;
                return ssa_build_ptradd(&self->builder, rhs, neg);
        }

        return ssa_build_sub(&self->builder, lhs, rhs);
}

static inline ssa_value* ssaize_assign(ssaizer* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_store(&self->builder, rhs, lhs)
                ? rhs : NULL;
}

static inline ssa_value* _ssaize_binary_expr(ssaizer*, tree_binop_kind, ssa_value*, ssa_value*);

static inline ssa_value* ssaize_compound_assign(ssaizer* self,
        tree_binop_kind prefix, ssa_value* lhs, ssa_value* rhs)
{
        ssa_value* val = ssa_build_load(&self->builder, lhs);
        if (!val)
                return NULL;

        tree_type* rhs_type = ssa_get_value_type(rhs);
        tree_type* lhs_type = tree_get_pointer_target(ssa_get_value_type(lhs));
        if (!tree_type_is_pointer(lhs_type))
                if (!(val = ssa_build_cast(&self->builder, rhs_type, val)))
                        return NULL;

        TREE_ASSERT_BINOP_KIND(prefix);
        ssa_value* result = _ssaize_binary_expr(self, prefix, val, rhs);
        if (!result)
                return NULL;

        if (!(result = ssa_build_cast(&self->builder, lhs_type, result)))
                return NULL;

        return ssaize_assign(self, lhs, result);
}

static inline ssa_value* _ssaize_binary_expr(
        ssaizer* self, tree_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        TREE_ASSERT_BINOP_KIND(opcode);
        if (opcode >= TBK_LE && opcode <= TBK_NEQ)
        {
                tree_type* lt = ssa_get_value_type(lhs);
                tree_type* rt = ssa_get_value_type(rhs);
                if (tree_type_is_pointer(lt) && ssa_get_value_kind(rhs) == SVK_CONSTANT)
                {
                        if (avalue_is_zero(ssa_get_constant_value(rhs)))
                                if (!(rhs = ssa_build_zero(&self->builder, lt)))
                                        return NULL;
                }
                else if (tree_type_is_pointer(rt) && ssa_get_value_kind(lhs) == SVK_CONSTANT)
                {
                        if (avalue_is_zero(ssa_get_constant_value(lhs)))
                                if (!(lhs = ssa_build_zero(&self->builder, rt)))
                                        return NULL;
                }
        }

        ssa_builder* b = &self->builder;
        switch (opcode)
        {
                case TBK_MUL:        return ssa_build_mul(b, lhs, rhs);
                case TBK_DIV:        return ssa_build_div(b, lhs, rhs);
                case TBK_MOD:        return ssa_build_mod(b, lhs, rhs);
                case TBK_ADD:        return ssaize_add(self, lhs, rhs);
                case TBK_SUB:        return ssaize_sub(self, lhs, rhs);
                case TBK_SHL:        return ssa_build_shl(b, lhs, rhs);
                case TBK_SHR:        return ssa_build_shr(b, lhs, rhs);
                case TBK_OR:         return ssa_build_or(b, lhs, rhs);
                case TBK_AND:        return ssa_build_and(b, lhs, rhs);
                case TBK_XOR:        return ssa_build_xor(b, lhs, rhs);     
                case TBK_LE:         return ssa_build_le(b, lhs, rhs);
                case TBK_LEQ:        return ssa_build_leq(b, lhs, rhs);
                case TBK_GR:         return ssa_build_gr(b, lhs, rhs);
                case TBK_GEQ:        return ssa_build_geq(b, lhs, rhs);  
                case TBK_EQ:         return ssa_build_eq(b, lhs, rhs);
                case TBK_NEQ:        return ssa_build_neq(b, lhs, rhs);
                case TBK_ASSIGN:     return ssaize_assign(self, lhs, rhs);
                case TBK_ADD_ASSIGN: return ssaize_compound_assign(self, TBK_ADD, lhs, rhs);
                case TBK_SUB_ASSIGN: return ssaize_compound_assign(self, TBK_SUB, lhs, rhs);
                case TBK_MUL_ASSIGN: return ssaize_compound_assign(self, TBK_MUL, lhs, rhs);
                case TBK_DIV_ASSIGN: return ssaize_compound_assign(self, TBK_DIV, lhs, rhs);
                case TBK_MOD_ASSIGN: return ssaize_compound_assign(self, TBK_MOD, lhs, rhs);
                case TBK_SHL_ASSIGN: return ssaize_compound_assign(self, TBK_SHL, lhs, rhs);
                case TBK_SHR_ASSIGN: return ssaize_compound_assign(self, TBK_SHR, lhs, rhs);
                case TBK_AND_ASSIGN: return ssaize_compound_assign(self, TBK_AND, lhs, rhs);
                case TBK_OR_ASSIGN:  return ssaize_compound_assign(self, TBK_OR, lhs, rhs);
                case TBK_XOR_ASSIGN: return ssaize_compound_assign(self, TBK_XOR, lhs, rhs);
                
                case TBK_COMMA:
                        return rhs;

                case TBK_LOG_AND:
                case TBK_LOG_OR:
                        S_ASSERT(0 && "Logical operations are not handled here");
                default:
                        S_ASSERT(0 && "Invalid binop kind");
                        return NULL;
        }
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
                        ssa_builder_gen_uid(self->builder));

        return self->exit;
}

static void ssa_br_expr_info_add_phi_var(
        ssa_br_expr_info* self, ssa_value* var, ssa_block* block)
{
        S_ASSERT(block);
        if (!self->phi)
        {
                ssa_block* current = ssa_builder_get_block(self->builder);
                ssa_builder_set_block(self->builder, ssa_br_expr_info_get_exit(self));
                self->phi = ssa_get_var_instr(ssa_build_phi(self->builder, self->phi_type));
                ssa_builder_set_block(self->builder, current);
        }
        ssa_add_phi_operand(self->phi,
                self->builder->context, var, ssa_get_block_label(block));
}

static inline ssa_value* ssaize_log_expr_lhs(
        ssaizer* self, ssa_br_expr_info* info, const tree_expr* expr)
{
        if (!tree_expr_is(expr, TEK_BINARY))
                return ssaize_expr_as_condition(self, expr);

        tree_binop_kind kind = tree_get_binop_kind(expr);
        S_ASSERT(kind == TBK_LOG_AND || kind == TBK_LOG_OR);

        ssa_value* lhs_cond = ssaize_log_expr_lhs(self, info, tree_get_binop_lhs(expr));
        ssa_block* lhs_cond_block = self->block;
        if (!lhs_cond)
                return NULL;

        ssa_block* current = ssaizer_new_block(self);
        ssa_block* exit = ssa_br_expr_info_get_exit(info);

        // finish prev block
        ssa_block* on_true = kind == TBK_LOG_OR ? exit : current;
        ssa_block* on_false = kind == TBK_LOG_OR ? current : exit;
        if (!ssaizer_build_if(self, lhs_cond, on_true, on_false))
                return false;

        ssa_br_expr_info_add_phi_var(info, lhs_cond, lhs_cond_block);
        ssaizer_enter_block(self, current);
        return ssaize_expr_as_condition(self, tree_get_binop_rhs(expr));;
}

static ssa_value* ssaize_log_expr(ssaizer* self, const tree_expr* expr)
{
        ssa_br_expr_info info;
        ssa_init_br_expr_info(&info, &self->builder, tree_get_expr_type(expr));

        ssa_value* last_cond = ssaize_log_expr_lhs(self, &info, expr);
        ssa_block* last_cond_block = self->block;
        if (!last_cond)
                return NULL;

        ssa_block* exit = ssa_br_expr_info_get_exit(&info);
        if (!ssaizer_build_jmp(self, exit))
                return false;

        ssa_br_expr_info_add_phi_var(&info, last_cond, last_cond_block);
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

static ssa_value* ssaize_inc(ssaizer* self, ssa_value* operand, bool prefix)
{
        ssa_value* val = ssa_build_load(&self->builder, operand);
        if (!val)
                return NULL;

        tree_type* val_type = ssa_get_value_type(val);
        ssa_value* one = tree_type_is_pointer(val_type)
                ? ssa_build_size_t_constant(&self->builder, 1)
                : ssa_build_one(&self->builder, val_type);
        if (!one)
                return NULL;

        ssa_value* inc = ssaize_add(self, val, one);
        if (!inc || !ssaize_assign(self, operand, inc))
                return NULL;

        return prefix ? inc : val;
}

static ssa_value* ssaize_dec(ssaizer* self, ssa_value* operand, bool prefix)
{
        ssa_value* val = ssa_build_load(&self->builder, operand);
        if (!val)
                return NULL;

        tree_type* val_type = ssa_get_value_type(val);
        ssa_value* one = tree_type_is_pointer(val_type)
                         ? ssa_build_size_t_constant(&self->builder, 1)
                         : ssa_build_one(&self->builder, val_type);
        if (!one)
                return NULL;

        ssa_value* dec = ssaize_sub(self, val, one);
        if (!dec || !ssaize_assign(self, operand, dec))
                return NULL;

        return prefix ? dec : val;
}

extern ssa_value* ssaize_unary_expr(ssaizer* self, const tree_expr* expr)
{
        ssa_value* operand = ssaize_expr(self, tree_get_unop_expr(expr));
        if (!operand)
                return NULL;

        tree_unop_kind k = tree_get_unop_kind(expr);
        switch (k)
        {
                case TUK_POST_INC:
                case TUK_PRE_INC:
                        return ssaize_inc(self, operand, k == TUK_PRE_INC);
                case TUK_POST_DEC:
                case TUK_PRE_DEC:
                        return ssaize_dec(self, operand, k == TUK_PRE_DEC);
                case TUK_MINUS:
                        return ssa_build_neg(&self->builder, operand);
                case TUK_NOT:
                        return ssa_build_not(&self->builder, operand);
                case TUK_LOG_NOT:
                        return ssa_build_log_not(&self->builder, operand);
                case TUK_DEREFERENCE:
                        return tree_expr_is_lvalue(expr)
                                ? operand
                                : ssa_build_load(&self->builder, operand);
                case TUK_ADDRESS:
                case TUK_PLUS:
                        return operand;

                default:
                        S_ASSERT(0 && "Invalid unary operator");
                        return NULL;
        }
}

extern ssa_value* ssaize_call_expr(ssaizer* self, const tree_expr* expr)
{
        ssa_value* func = ssaize_expr(self, tree_get_call_lhs(expr));
        if (!func)
                return NULL;

        dseq args;
        dseq_init_ex_ptr(&args, ssa_get_alloc(self->context));

        TREE_FOREACH_CALL_ARG(expr, it)
        {
                ssa_value* arg = ssaize_expr(self, *it);
                if (!arg)
                {
                        dseq_dispose(&args);
                        return NULL;
                }

                dseq_append_ptr(&args, arg);
        }

        ssa_value* call = ssa_build_call(&self->builder, func, &args);
        dseq_dispose(&args);
        return call;
}

extern ssa_value* ssaize_subscript_expr(ssaizer* self, const tree_expr* expr)
{
        tree_expr* pointer = tree_get_subscript_lhs(expr);
        tree_expr* index = tree_get_subscript_rhs(expr);

        // 0[pointer]
        if (tree_type_is_pointer(tree_get_expr_type(index)))
        {
                tree_expr* tmp = pointer;
                pointer = index;
                index = tmp;
        }

        ssa_value* ssa_pointer = ssaize_expr(self, pointer);
        if (!ssa_pointer)
                return NULL;

        ssa_value* ssa_index = ssaize_expr(self, index);
        if (!ssa_index)
                return NULL;
        
        ssa_value* element_ptr = ssa_build_ptradd(&self->builder, ssa_pointer, ssa_index);
        if (!element_ptr)
                return NULL;

        return tree_expr_is_lvalue(expr)
                ? element_ptr
                : ssa_build_load(&self->builder, element_ptr);
}

static bool ssaize_conditional_expr_branch(
        ssaizer* self, ssa_br_expr_info* info, const tree_expr* branch)
{
        ssa_value* val = ssaize_expr(self, branch);
        if (!val)
                return false;

        ssa_br_expr_info_add_phi_var(info, val, self->block);
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
        tree_type* type = tree_get_expr_type(expr);
        const float_value* value = tree_get_floating_literal_cvalue(expr);

        return tree_builtin_type_is(type, TBTK_FLOAT)
                ? ssa_build_sp_constant(&self->builder, type, float_get_sp(value))
                : ssa_build_dp_constant(&self->builder, type, float_get_dp(value));
}

extern ssa_value* ssaize_string_literal(ssaizer* self, const tree_expr* expr)
{
        ssa_value* string = ssa_build_string(&self->builder,
                tree_get_expr_type(expr), tree_get_string_literal(expr));
        if (!string)
                return NULL;

        ssa_module_add_global(self->module, string);
        return string;
}

static ssa_value* ssaizer_get_global_decl_ptr(ssaizer* self, tree_decl* decl)
{
        ssa_value* global = ssaizer_get_global_decl(self, decl);
        if (!global)
        {
                tree_type* type = tree_get_decl_type(decl);
                if (!tree_type_is(type, TTK_FUNCTION) && !tree_type_is(type, TTK_ARRAY))
                        type = tree_new_pointer_type(ssa_get_tree(self->context), type);
                global = ssa_new_decl(self->context, type, decl);
                ssaizer_set_global_decl(self, decl, global);
        }
        return global;
}

extern ssa_value* ssaize_decl_expr(ssaizer* self, const tree_expr* expr)
{
        tree_decl* decl = tree_get_decl_expr_entity(expr);
        if (!decl)
                return NULL;
        
        ssa_value* def = tree_decl_is_global(decl)
                ? ssaizer_get_global_decl_ptr(self, decl)
                : ssaizer_get_def(self, decl);
        if (!def)
                return NULL;

        return tree_expr_is_lvalue(expr) || tree_decl_is(decl, TDK_FUNCTION)
                ? def
                : ssa_build_load(&self->builder, def);
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
                return ssa_build_neq_zero(&self->builder, v);

        ssa_instr* i = ssa_get_var_instr(v);
        if (ssa_get_instr_kind(i) != SIK_BINARY)
                return ssa_build_neq_zero(&self->builder, v);

        switch (ssa_get_binop_kind(i))
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