#include "scc/c/c-sema-expr.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-conv.h"
#include "c-misc.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-eval.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-target.h"
#include "c-initializer.h"

extern bool c_sema_require_object_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_object_pointer(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_pointer_to_object_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_function_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_function_pointer(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_pointer_to_function_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_integral_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_integer(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_integral_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_integer_expr(const c_sema* self, const tree_expr* e)
{
        return c_sema_require_integral_expr_type(self,
                tree_get_expr_type(e), tree_get_expr_loc(e));
}

extern bool c_sema_require_real_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_real(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_real_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_record_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_record(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_record_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_array_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_array(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_array_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_scalar_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_scalar(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_scalar_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_scalar_expr(const c_sema* self, const tree_expr* e)
{
        return c_sema_require_scalar_expr_type(self,
                tree_get_expr_type(e), tree_get_expr_loc(e));
}

extern bool c_sema_require_arithmetic_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_arithmetic(tree_desugar_ctype(t)))
        {
                c_error_expr_must_have_arithmetic_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_real_or_object_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l)
{
        t = tree_desugar_ctype(t);
        if (!tree_type_is_real(t) && !tree_type_is_pointer(t))
        {
                c_error_expr_must_have_real_or_pointer_to_object_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool c_sema_require_lvalue_or_function_designator(
        const c_sema* self, const tree_expr* e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(e));
        if (!tree_expr_is_lvalue(e) && tree_get_type_kind(t) != TTK_FUNCTION)
        {
                c_error_expr_must_be_lvalue_or_function_designator(self->logger, e);
                return false;
        }
        return true;
}

extern bool c_sema_require_modifiable_lvalue(const c_sema* self, const tree_expr* e)
{
        if (!tree_expr_is_modifiable_lvalue(e))
        {
                c_error_expr_must_be_modifiable_lvalue(self->logger, e);
                return false;
        }
        return true;
}

extern bool c_sema_require_compatible_expr_types(
        const c_sema* self, const tree_type* a, const tree_type* b, tree_location l, bool unqualify)
{
        if (!c_sema_types_are_compatible(self, a, b, unqualify))
        {
                c_error_types_are_not_compatible(self->logger, l);
                return false;
        }
        return true;
}

extern tree_expr* c_sema_new_paren_expr(
        c_sema* self, tree_location lbracket_loc, tree_expr* expr, tree_location rbracket_loc)
{
        if (!expr)
                return NULL;

        return tree_new_paren_expr(
                self->context,
                tree_get_expr_value_kind(expr),
                tree_get_expr_type(expr),
                lbracket_loc,
                expr);
}

static bool c_sema_check_object_type(c_sema* self, tree_location loc, tree_type* type)
{
        if (c_sema_in_transaction_safe_block(self) && (tree_get_type_quals(type) & TTQ_VOLATILE))
        {
                c_error_reffering_volatile_object_is_not_allowed(
                        self->logger, loc, c_sema_in_atomic_block(self));
                return false;
        }
        return true;
}

extern tree_expr* c_sema_new_decl_expr(c_sema* self, tree_id id, tree_location id_loc)
{
        tree_decl* d = c_sema_require_local_decl(self, id_loc, id);
        if (!d)
                return NULL; // unknown decl

        tree_decl_kind dk = tree_get_decl_kind(d);
        if (dk != TDK_VAR && dk != TDK_FUNCTION && dk != TDK_ENUMERATOR && dk != TDK_PARAM)
        {
                c_error_undeclared_identifier(self->logger, id_loc, id);
                return NULL;
        }

        tree_value_kind vk = TVK_LVALUE;
        tree_type* t = tree_desugar_type(tree_get_decl_type(d));

        if (!c_sema_check_object_type(self, id_loc, t))
                return NULL;

        if (dk == TDK_ENUMERATOR || dk == TDK_FUNCTION)
                vk = TVK_RVALUE;

        return tree_new_decl_expr(self->context, vk, t, id_loc, d);
}

extern tree_expr* c_sema_new_sp_floating_literal(c_sema* self, tree_location loc, float v)
{
        float_value value;
        float_init_sp(&value, v);

        return tree_new_floating_literal(self->context,
                c_sema_get_float_type(self), loc, &value);
}

extern tree_expr* c_sema_new_dp_floating_literal(c_sema* self, tree_location loc, ldouble v)
{
        float_value value;
        float_init_dp(&value, v);

        return tree_new_floating_literal(self->context,
                c_sema_get_double_type(self), loc, &value);
}

extern tree_expr* c_sema_new_character_literal(c_sema* self, tree_location loc, int c)
{
        return tree_new_character_literal(self->context, c_sema_get_char_type(self), loc, c);
}

extern tree_expr* c_sema_new_integer_literal(
        c_sema* self, tree_location loc, uint64_t v, bool signed_, bool ext)
{
        return tree_new_integer_literal(self->context,
                c_sema_get_int_type(self, signed_, ext), loc, v);
}

extern tree_expr* c_sema_new_string_literal(c_sema* self, tree_location loc, tree_id ref)
{
        return tree_new_string_literal(self->context, TVK_LVALUE,
                c_sema_get_type_for_string_literal(self, ref), loc, ref);
}

// c99 6.5.2.1 array subscripting
// One of the expressions shall have type "pointer to object type",
// the other expression shall have integer type, and the result has type "type".
extern tree_expr* c_sema_new_subscript_expr(
        c_sema* self, tree_location loc, tree_expr* lhs, tree_expr* rhs)
{
        if (!lhs || !rhs)
                return NULL;

        c_sema_unary_conversion(self, &lhs);
        tree_type* rt = c_sema_unary_conversion(self, &rhs);

        tree_expr* base = lhs;
        tree_expr* index = rhs;
        if (tree_type_is_pointer(rt))
        {
                base = rhs;
                index = lhs;
        }

        tree_type* base_type = tree_get_expr_type(base);
        if (!tree_type_is_object_pointer(base_type))
        {
                c_error_subscripted_value_isnt_array(self->logger, loc);
                return NULL;
        }
        if (!c_sema_require_integer_expr(self, index))
                return NULL;

        tree_type* eltype = tree_get_pointer_target(base_type);
        if (!c_sema_check_object_type(self, loc, eltype))
                return NULL;

        return tree_new_subscript_expr(self->context, TVK_LVALUE, eltype, loc, lhs, rhs);
}

static bool c_sema_check_call_argument(
        c_sema* self, tree_type* arg_type, tree_expr** arg, uint pos)
{
        c_assignment_conversion_result r;
        if (!c_sema_assignment_conversion(self, arg_type, arg, &r))
        {
                c_error_invalid_argument_assignment(self->logger, tree_get_expr_loc(*arg), pos, &r);
                return false;
        }
        return true;
}

// c99 6.5.2.2 function calls
// The expression that denotes the called function shall have type pointer to function
// returning void or returning an object type other than an array type.
// If the expression that denotes the called function has a type that includes a prototype,
// the number of arguments shall agree with the number of parameters. Each argument shall
// have a type such that its value may be assigned to an object with the unqualified version
// of the type of its corresponding parameter.
extern tree_expr* c_sema_new_call_expr(
        c_sema* self, tree_location loc, tree_expr* lhs)
{
        if (!lhs)
                return NULL;

        tree_type* t = c_sema_unary_conversion(self, &lhs);
        if (!c_sema_require_function_pointer_expr_type(self, t, tree_get_expr_loc(lhs)))
                return NULL;

        tree_type* ft = tree_desugar_type(tree_get_pointer_target(tree_desugar_ctype(t)));
        if (c_sema_in_transaction_safe_block(self) && !tree_func_type_is_transaction_safe(ft))
        {
                c_error_transaction_unsafe_function_is_not_allowed(
                        self->logger, loc, c_sema_in_atomic_block(self));
                return NULL;
        }

        tree_type* restype = tree_get_func_type_result(ft);
        if (!c_sema_check_object_type(self, loc, restype))
                return NULL;

        return tree_new_call_expr(self->context, TVK_RVALUE, restype, loc, lhs);
}

extern void c_sema_add_call_expr_arg(c_sema* self, tree_expr* call, tree_expr* arg)
{
        assert(call && arg);
        tree_add_call_arg(call, self->context, arg);
}

extern tree_expr* c_sema_check_call_expr_args(c_sema* self, tree_expr* call)
{
        assert(call);

        tree_expr* lhs = tree_get_call_lhs(call);
        tree_type* ft = tree_desugar_type(tree_get_pointer_target(tree_get_expr_type(lhs)));
        size_t num_params = tree_get_func_type_params_size(ft);
        size_t num_args = tree_get_call_args_size(call);

        if (num_args < num_params)
        {
                c_error_too_few_arguments(self->logger, lhs);
                return NULL;
        }
        if (!tree_func_type_is_vararg(ft) && num_args > num_params)
        {
                c_error_too_many_arguments(self->logger, lhs);
                return NULL;
        }

        size_t i = 0;
        for (; i < num_params; i++)
        {
                tree_expr** parg = tree_get_call_args_begin(call) + i;
                tree_type* arg_type = tree_get_func_type_param(ft, i);
                if (!c_sema_check_call_argument(self, arg_type, parg, i + 1))
                        return NULL;
        }

        for (; i < num_args; i++)
                c_sema_default_argument_promotion(self, tree_get_call_args_begin(call) + i);

        return call;
}

// c99 6.5.2.3 structure and union members
// The first operand of the . operator shall have a qualified or unqualified
// structure or union type, and the second operand shall name a member of that type.
// The first operand of the -> operator shall have type "pointer to qualified 
// or unqualified structure" or "pointer to qualified or unqualified union",
// and the second operand shall name a member of the type pointed to.
extern tree_expr* c_sema_new_member_expr(
        c_sema* self,
        tree_location loc,
        tree_expr* lhs,
        tree_id id,
        tree_location id_loc,
        bool is_arrow)
{
        if (!lhs)
                return NULL;

        tree_location lhs_loc = tree_get_expr_loc(lhs);
        tree_type* lhs_type = c_sema_array_function_to_pointer_conversion(self, &lhs);

        if (is_arrow)
        {
                lhs_type = c_sema_lvalue_conversion(self, &lhs);
                if (!c_sema_require_object_pointer_expr_type(self, lhs_type, lhs_loc))
                        return NULL;

                lhs_type = tree_get_pointer_target(lhs_type);
        }
        lhs_type = tree_desugar_type(lhs_type);
        if (!c_sema_require_record_expr_type(self, lhs_type, lhs_loc))
                return NULL;

        tree_decl* record = tree_get_decl_type_entity(lhs_type);
        tree_decl* field = c_sema_require_field_decl(self, record, id_loc, id);
        if (!field)
                return NULL;

        tree_type* field_type = tree_get_decl_type(field);
        if (!c_sema_check_object_type(self, id_loc, field_type))
                return NULL;

        if (tree_decl_is(field, TDK_FIELD))
                return tree_new_member_expr(self->context,
                        TVK_LVALUE, tree_get_decl_type(field), loc, lhs, field, is_arrow);

        tree_decl* anon = tree_get_indirect_field_anon_record(field);
        lhs = tree_new_member_expr(self->context,
                TVK_LVALUE, tree_get_decl_type(anon), loc, lhs, anon, is_arrow);

        return c_sema_new_member_expr(self, loc, lhs, id, id_loc, false);
}

// c99 6.5.2.4/6.5.3.1
// The operand of the postfix (or prefix) increment or decrement operator shall have
// qualified or unqualified real or pointer type and shall be a modifiable lvalue.
static tree_type* c_sema_check_inc_dec_expr(c_sema* self, tree_expr** expr)
{
        tree_type* t = c_sema_array_function_to_pointer_conversion(self, expr);
        if (!c_sema_require_real_or_object_pointer_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;
        if (!c_sema_require_modifiable_lvalue(self, *expr))
                return NULL;
        return t;
}

// 6.5.3.2 address and inderection operators
// The operand of the unary & operator shall be either a function designator, the result of a
// [] or unary * operator, or an lvalue that designates an object
// that is not a bit - field and is not declared with the register storage - class specifier.
static tree_type* c_sema_check_address_expr(c_sema* self, tree_expr** expr)
{
        if (!c_sema_require_lvalue_or_function_designator(self, *expr))
                return NULL;

        return c_sema_new_pointer_type(self, TTQ_UNQUALIFIED, tree_get_expr_type(*expr));
}

// 6.5.3.2 address and inderection operators
// The operand of the unary * operator shall have pointer type.
static tree_type* c_sema_check_dereference_expr(c_sema* self, tree_expr** expr, tree_value_kind* vk)
{
        tree_type* t = tree_desugar_type(c_sema_unary_conversion(self, expr));
        tree_location loc = tree_get_expr_loc(*expr);
        if (!c_sema_require_object_pointer_expr_type(self, t, loc))
                return NULL;

        t = tree_get_pointer_target(t);
        if (!c_sema_check_object_type(self, loc, t))
                return NULL;

        *vk = TVK_LVALUE;
        return t;
}

// 6.5.3.3 unary arithmetic
// The operand of the ! operator shall have scalar type
static tree_type* c_sema_check_log_not_expr(c_sema* self, tree_expr** expr)
{
        tree_type* t = c_sema_unary_conversion(self, expr);
        if (!c_sema_require_scalar_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return c_sema_get_logical_operation_type(self);
}

// 6.5.3.3 unary arithmetic
// The operand of the unary + or - operator shall have arithmetic type
static tree_type* c_sema_check_plus_minus_expr(c_sema* self, tree_expr** expr)
{
        tree_type* t = c_sema_unary_conversion(self, expr);
        if (!c_sema_require_arithmetic_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return c_sema_integer_promotion(self, expr);
}

// 6.5.3.3 unary arithmetic
// The operand of the ~ operator shall have integer type
static tree_type* c_sema_check_not_expr(c_sema* self, tree_expr** expr)
{
        tree_type* t = c_sema_unary_conversion(self, expr);
        if (!c_sema_require_integral_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return c_sema_integer_promotion(self, expr);
}

extern tree_expr* c_sema_new_unary_expr(
        c_sema* self, tree_location loc, tree_unop_kind opcode, tree_expr* expr)
{
        if (!expr)
                return NULL;

        tree_type* t = NULL;
        tree_value_kind vk = TVK_RVALUE;

        switch (opcode)
        {
                case TUK_POST_INC:
                case TUK_POST_DEC:
                case TUK_PRE_INC:
                case TUK_PRE_DEC:
                        t = c_sema_check_inc_dec_expr(self, &expr);
                        break;

                case TUK_ADDRESS:
                        t = c_sema_check_address_expr(self, &expr);
                        break;

                case TUK_DEREFERENCE:
                        t = c_sema_check_dereference_expr(self, &expr, &vk);
                        break;

                case TUK_LOG_NOT:
                        t = c_sema_check_log_not_expr(self, &expr);
                        break;

                case TUK_PLUS:
                case TUK_MINUS:
                        t = c_sema_check_plus_minus_expr(self, &expr);
                        break;

                case TUK_NOT:
                        t = c_sema_check_not_expr(self, &expr);
                        break;

                default:
                        // unknown unop
                        UNREACHABLE();
        }
        if (!t)
                return NULL;

        return tree_new_unop(self->context, vk, t, loc, opcode, expr);
}

// c99 6.5.3.4
// The sizeof operator shall not be applied to an expression that has function type or an
// incomplete type, to the parenthesized name of such a type, or to an expression that
// designates a bit - field member.
extern tree_expr* c_sema_new_sizeof_expr(
        c_sema* self, tree_location loc, void* operand, bool contains_type)
{
        if (!operand)
                return NULL;

        tree_type* type = contains_type ? operand : tree_get_expr_type(operand);
        if (tree_type_is(type, TTK_FUNCTION))
        {
                c_error_operand_of_sizeof_is_function(self->logger, loc);
                return NULL;
        }
        if (!c_sema_require_complete_type(self, loc, type))
                return NULL;
        if (!contains_type && tree_expr_designates_bitfield(operand))
        {
                c_error_operand_of_sizeof_is_bitfield(self->logger, loc);
                return NULL;
        }

        return tree_new_sizeof_expr(self->context,
                c_sema_get_size_t_type(self), loc, operand, contains_type);
}

// c99 6.5.4 cast operators
// Unless the type name specifies a void type, the type name shall specify qualified or
// unqualified scalar type and the operand shall have scalar type.
// Conversions that involve pointers, other than where permitted by the constraints of
// 6.5.16.1, shall be specified by means of an explicit cast.
extern tree_expr* c_sema_new_cast_expr(
        c_sema* self, tree_location loc, tree_type* type, tree_expr* expr)
{
        if (!type || !expr)
                return NULL;

        tree_type* et = tree_desugar_type(c_sema_unary_conversion(self, &expr));
        if (!tree_type_is_void(type))
        {
                if (!c_sema_require_scalar_expr_type(self, type, loc))
                        return NULL;
                if (!c_sema_check_object_type(self, loc, type))
                        return NULL;
                if (!c_sema_require_scalar_expr_type(self, et, tree_get_expr_loc(expr)))
                        return NULL;
        }

        tree_value_kind vk = tree_get_expr_value_kind(expr);
        return tree_new_cast_expr(self->context, vk, loc, type, expr, false);
}

// 6.5.5 multiplicative
// Each of the operands shall have arithmetic type.
static tree_type* c_sema_check_mul_div_expr(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);

        if (!c_sema_require_arithmetic_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;
        if (!c_sema_require_arithmetic_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;

        return c_sema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
}

// 6.5.7/6.5.10-13
// Each of the operands shall have integer type.
static tree_type* c_sema_check_bitwise_expr(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);

        if (!c_sema_require_integral_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;
        if (!c_sema_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;

        return c_sema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
}

// 6.5.13/6.5.14 logical and/or operator
// Each of the operands shall have scalar type
static tree_type* c_sema_check_log_expr(
        c_sema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);

        if (!c_sema_require_scalar_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;
        if (!c_sema_require_scalar_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;

        return c_sema_get_logical_operation_type(self);
}

// 6.5.8 Relational operators
// One of the following shall hold:
// - both operands have real type;
// - both operands are pointers to qualified or unqualified versions of
// compatible object types; or
// - both operands are pointers to qualified or unqualified versions of
// compatible incomplete types
static tree_type* c_sema_check_relational_expr(
        c_sema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_real(lt) && tree_type_is_real(rt))
        {
                if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                        c_sema_usual_arithmetic_conversion(self, lhs, rhs, true);
        }
        else if (tree_type_is_object_pointer(lt) && tree_type_is_object_pointer(rt))
        {
                tree_type* ltarget = tree_get_pointer_target(lt);
                tree_type* rtarget = tree_get_pointer_target(rt);
                if (!c_sema_require_compatible_expr_types(self, ltarget, rtarget, loc, true))
                        return NULL;
        }
        else
        {
                c_error_invalid_binop_operands(self->logger, loc, opcode);
                return NULL;
        }

        return c_sema_get_logical_operation_type(self);
}

// 6.5.9 Equality operators
// One of the following shall hold:
// - both operands have arithmetic type;
// - both operands are pointers to qualified or unqualified versions of compatible types;
// - one operand is a pointer to an object or incomplete type and the other is a pointer to a
// qualified or unqualified version of void; or
// - one operand is a pointer and the other is a null pointer constant.
static tree_type* c_sema_check_compare_expr(
        c_sema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                c_sema_usual_arithmetic_conversion(self, lhs, rhs, true);
        else if (tree_type_is_pointer(lt)
                && tree_expr_is_null_pointer_constant(self->context, *rhs))
        {
                ;
        }
        else if (tree_type_is_pointer(rt)
                && tree_expr_is_null_pointer_constant(self->context, *lhs))
        {
                ;
        }
        else if (tree_type_is_pointer(lt) && tree_type_is_pointer(rt))
        {
                tree_type* ltarget = tree_desugar_type(tree_get_pointer_target(lt));
                tree_type* rtarget = tree_desugar_type(tree_get_pointer_target(rt));

                if ((tree_type_is_incomplete(ltarget) && !tree_type_is_void(rtarget))
                        || (tree_type_is_incomplete(rtarget) && !tree_type_is_void(ltarget)))
                {
                        c_error_cmp_of_distinct_pointers(self->logger, loc);
                        return NULL;
                }
                else if (!c_sema_require_compatible_expr_types(self, ltarget, rtarget, loc, true))
                {
                        return NULL;
                }
        }
        else
        {
                c_error_invalid_binop_operands(self->logger, loc, opcode);
                return NULL;
        }
        return c_sema_get_logical_operation_type(self);
}

// 6.5.5 multiplicative
// The operands of the % operator shall have integer type
static tree_type* c_sema_check_mod_expr(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);

        if (!c_sema_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;
        if (!c_sema_require_integral_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;

        return c_sema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
}

// 6.5.6 additive
// For addition, either both operands shall have arithmetic type, or one operand shall be a
// pointer to an object type and the other shall have integer type.
// (Incrementing is equivalent to adding 1.)
static tree_type* c_sema_check_add_expr(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return c_sema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
        else if (tree_type_is_object_pointer(lt))
        {
                if (!c_sema_require_integral_expr_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        else if (tree_type_is_object_pointer(rt))
        {
                if (!c_sema_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                        return NULL;

                return rt;
        }
        c_error_invalid_binop_operands(self->logger, loc, is_assign ? TBK_ADD_ASSIGN : TBK_ADD);
        return NULL;
}

// 6.5.6 additive
// For subtraction, one of the following shall hold :
// - both operands have arithmetic type;
// - both operands are pointers to qualified or unqualified versions
// of compatible object types; or
// - the left operand is a pointer to an object type and the right operand has integer type.
// (Decrementing is equivalent to subtracting 1.)
static tree_type* c_sema_check_sub_expr(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign ? tree_get_expr_type(*lhs) : c_sema_unary_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return c_sema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
        else if (tree_type_is_object_pointer(lt))
        {
                if (tree_type_is_object_pointer(rt)
                        && c_sema_types_are_compatible(self,
                                tree_get_pointer_target(lt), tree_get_pointer_target(rt), true))
                {
                        return tree_get_ptrdiff_type(self->context);
                }

                if (!c_sema_require_integral_expr_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        c_error_invalid_binop_operands(self->logger, loc, is_assign ? TBK_SUB_ASSIGN : TBK_SUB);
        return NULL;
}

// 6.5.16.1 Simple assignment
// One of the following shall hold:
// - the left operand has qualified or unqualified arithmetic type and the right has
// arithmetic type;
// - the left operand has a qualified or unqualified version of a structure or union type
// compatible with the type of the right;
// - both operands are pointers to qualified or unqualified versions of compatible types,
// and the type pointed to by the left has all the qualifiers of the type pointed to by the
// right;
// - one operand is a pointer to an object or incomplete type and the other is a pointer to a
// qualified or unqualified version of void, and the type pointed to by the left has all
// the qualifiers of the type pointed to by the right;
// - the left operand is a pointer and the right is a null pointer constant; or
// - the left operand has type _Bool and the right is a pointer.
static tree_type* c_sema_check_assign_expr(
        c_sema* self,
        tree_expr** lhs,
        tree_expr** rhs,
        tree_location loc,
        tree_type* compound_type)
{
        tree_type* lt = c_sema_array_function_to_pointer_conversion(self, lhs);
        if (!c_sema_require_modifiable_lvalue(self, *lhs))
                return NULL;

        c_assignment_conversion_result r;
        tree_type* t = c_sema_assignment_conversion(self, compound_type ? compound_type : lt, rhs, &r);
        if (t)
                return t;

        c_error_invalid_assignment(self->logger, loc, *rhs, &r);
        return NULL;
}

// 6.5.16.2 Compound assignment
// For the operators += and -= only, either the left operand shall be a pointer to an object
// type and the right shall have integer type, or the left operand shall have qualified or
// unqualified arithmetic type and the right shall have arithmetic type.
static tree_type* c_sema_check_add_sub_assign_expr(
        c_sema* self, tree_binop_kind opcode, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        tree_type* lt = c_sema_array_function_to_pointer_conversion(self, lhs);
        tree_type* rt = c_sema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (!c_sema_require_modifiable_lvalue(self, *lhs))
                return NULL;

        if (tree_type_is_object_pointer(lt))
        {
                if (!c_sema_require_integral_expr_type(self, rt, rl))
                        return NULL;
        }
        else if (tree_type_is_arithmetic(lt))
        {
                if (!c_sema_require_arithmetic_expr_type(self, rt, rl))
                        return NULL;
        }
        else
        {
                c_error_invalid_binop_operands(self->logger, loc, opcode);
                return NULL;
        }

        return lt;
}

// returns type of the binary operator
static inline tree_type* c_sema_check_binop(
        c_sema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        if (!lhs || !rhs)
                return NULL;

        tree_type* compound;
        switch (opcode)
        {
                case TBK_MUL:
                case TBK_DIV:
                        return c_sema_check_mul_div_expr(self, lhs, rhs, loc, false);
                case TBK_MOD:
                        return c_sema_check_mod_expr(self, lhs, rhs, loc, false);
                case TBK_ADD:
                        return c_sema_check_add_expr(self, lhs, rhs, loc, false);
                case TBK_SUB:
                        return c_sema_check_sub_expr(self, lhs, rhs, loc, false);
                case TBK_SHL:
                case TBK_SHR:
                case TBK_OR:
                case TBK_AND:
                case TBK_XOR:
                        return c_sema_check_bitwise_expr(self, lhs, rhs, loc, false);
                case TBK_LE:
                case TBK_LEQ:
                case TBK_GR:
                case TBK_GEQ:
                        return c_sema_check_relational_expr(self, opcode, loc, lhs, rhs);
                case TBK_EQ:
                case TBK_NEQ:
                        return c_sema_check_compare_expr(self, opcode, loc, lhs, rhs);
                case TBK_LOG_AND:
                case TBK_LOG_OR:
                        return c_sema_check_log_expr(self, opcode, loc, lhs, rhs);
                case TBK_ASSIGN:
                        return c_sema_check_assign_expr(self, lhs, rhs, loc, NULL);
                case TBK_ADD_ASSIGN:
                        if (!(c_sema_check_add_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return c_sema_check_add_sub_assign_expr(self, opcode, lhs, rhs, loc);
                case TBK_SUB_ASSIGN:
                        if (!(c_sema_check_sub_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return c_sema_check_add_sub_assign_expr(self, opcode, lhs, rhs, loc);
                case TBK_MUL_ASSIGN:
                case TBK_DIV_ASSIGN:
                        if (!(compound = c_sema_check_mul_div_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return c_sema_check_assign_expr(self, lhs, rhs, loc, compound);
                case TBK_MOD_ASSIGN:
                        if (!(compound = c_sema_check_mod_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return c_sema_check_assign_expr(self, lhs, rhs, loc, compound);
                case TBK_SHL_ASSIGN:
                case TBK_SHR_ASSIGN:
                case TBK_AND_ASSIGN:
                case TBK_OR_ASSIGN:
                case TBK_XOR_ASSIGN:
                        if (!(compound = c_sema_check_bitwise_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return c_sema_check_assign_expr(self, lhs, rhs, loc, compound);
                case TBK_COMMA:
                        c_sema_unary_conversion(self, lhs);
                        return c_sema_unary_conversion(self, rhs);
                default:
                        assert(0 && "Invalid binop kind");
                        return NULL;

        }
}

extern tree_expr* c_sema_new_binary_expr(
        c_sema* self, tree_location loc, tree_binop_kind opcode, tree_expr* lhs, tree_expr* rhs)
{
        if (!lhs || !rhs)
                return NULL;

        tree_type* t = c_sema_check_binop(self, opcode, loc, &lhs, &rhs);
        if (!t)
                return NULL;

        return tree_new_binop(self->context, TVK_RVALUE, t, loc, opcode, lhs, rhs);
}

static tree_type* c_sema_check_conditional_operator_pointer_types(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        tree_type* lt = tree_get_expr_type(*lhs);
        tree_type* rt = tree_get_expr_type(*rhs);
        bool lpointer = tree_type_is_object_pointer(lt);
        bool rpointer = tree_type_is_object_pointer(rt);

        tree_type* target = NULL;
        tree_type_quals quals = TTQ_UNQUALIFIED;
        if (rpointer && tree_expr_is_null_pointer_constant(self->context, *lhs))
        {
                target = tree_get_pointer_target(rt);
                quals = tree_get_type_quals(target);
                if (lpointer)
                        quals |= tree_get_type_quals(tree_get_pointer_target(lt));
        }
        else if (lpointer && tree_expr_is_null_pointer_constant(self->context, *rhs))
        {
                target = tree_get_pointer_target(lt);
                quals = tree_get_type_quals(target);
                if (rpointer)
                        quals |= tree_get_type_quals(tree_get_pointer_target(rt));
        }
        else if (lpointer && rpointer)
        {
                tree_type* ltarget = tree_get_pointer_target(lt);
                tree_type* rtarget = tree_get_pointer_target(rt);
                if (tree_type_is_void(ltarget))
                        target = ltarget;
                else if (tree_type_is_void(rtarget))
                        target = rtarget;
                else if (c_sema_types_are_compatible(self, ltarget, rtarget, true))
                {
                        target = ltarget;
                }
                else
                {
                        c_error_pointer_type_mismatch(self->logger, loc);
                        return NULL;
                }
                quals = tree_get_type_quals(ltarget) | tree_get_type_quals(rtarget);
        }
        else
                return NULL;

        target = tree_new_qualified_type(self->context, target, quals);
        tree_type* result = c_sema_new_pointer_type(self, TTQ_UNQUALIFIED, target);
        *lhs = c_sema_new_impl_cast(self, *lhs, result);
        *rhs = c_sema_new_impl_cast(self, *rhs, result);

        return result;
}

// 6.5.15 Conditional operator
// The first operand shall have scalar type.
// One of the following shall hold for the second and third operands:
// - both operands have arithmetic type;
// - both operands have the same structure or union type;
// - both operands have void type;
// - both operands are pointers to qualified or unqualified versions of compatible types;
// - one operand is a pointer and the other is a null pointer constant; or
// - one operand is a pointer to an object or incomplete type and the other is a pointer to a
// qualified or unqualified version of void.
extern tree_expr* c_sema_new_conditional_expr(
        c_sema* self,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs)
{
        if (!condition || !lhs || !rhs)
                return NULL;

        tree_type* ct = c_sema_unary_conversion(self, &condition);
        if (!c_sema_require_scalar_expr_type(self, ct, tree_get_expr_loc(condition)))
                return NULL;

        tree_type* t = NULL;
        tree_type* lt = c_sema_unary_conversion(self, &lhs);
        tree_type* rt = c_sema_unary_conversion(self, &rhs);
        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                t = c_sema_usual_arithmetic_conversion(self, &lhs, &rhs, true);
        else if (tree_type_is_record(lt) && tree_type_is_record(rt))
        {
                if (!c_sema_require_compatible_expr_types(self, lt, rt, loc, false))
                        return NULL;
                t = lt;
        }
        else if (tree_type_is_void(lt) && tree_type_is_void(rt))
                t = lt;
        else if (!(t = c_sema_check_conditional_operator_pointer_types(self, &lhs, &rhs, loc)))
                return NULL;

        assert(t);
        return tree_new_conditional_expr(self->context, TVK_RVALUE, t, loc, condition, lhs, rhs);
}

extern tree_expr* c_sema_finish_expr(c_sema* self, tree_expr* expr)
{
        if (!expr)
                return NULL;

        c_sema_unary_conversion(self, &expr);
        return expr;
}

extern tree_expr* c_sema_new_initializer_list(c_sema* self, tree_location loc)
{
        return tree_new_init_list_expr(self->context, loc);
}

extern tree_expr* c_sema_add_initializer_list_expr(
        c_sema* self, tree_expr* list, tree_expr* expr)
{
        assert(expr);
        tree_add_init_list_expr(list, self->context, expr);
        return list;
}

static c_initialized_object* c_sema_check_field_designator(
        c_sema* self, tree_designator* designator, c_initialized_object* parent)
{
        if (parent->kind != CIOK_STRUCT && parent->kind != CIOK_UNION)
        {
                c_error_field_name_not_in_record_initializer(self->logger, designator);
                return NULL;
        }

        tree_decl* rec = tree_get_decl_type_entity(parent->type);
        tree_id name = tree_get_designator_field(designator);
        tree_id loc = tree_get_designator_loc(designator);
        tree_decl* field = c_sema_require_field_decl(self, rec, loc, name);
        if (!field)
                return NULL;

        while (tree_decl_is(field, TDK_INDIRECT_FIELD))
        {
                tree_decl* anon_field = tree_get_indirect_field_anon_record(field);
                rec = tree_get_decl_type_entity(tree_get_decl_type(anon_field));
                field = tree_decl_scope_lookup(tree_get_record_fields(rec), TLK_DECL, name, false);
                parent = c_initialized_object_new_rec(self->ccontext, rec, field, true, parent);
        }

        c_initialized_object_set_field(parent, field);
        return c_initialized_object_new(self->ccontext,
                c_initialized_object_get_subobject(parent), true, parent);
}

static c_initialized_object* c_sema_check_array_designator(
        c_sema* self, tree_designator* designator, c_initialized_object* parent)
{
        if (parent->kind != CIOK_ARRAY)
        {
                c_error_array_index_in_non_array_intializer(self->logger, designator);
                return false;
        }

        tree_expr* index = tree_get_designator_index(designator);
        tree_eval_result eval_result;
        if (!tree_eval_expr_as_integer(self->context, index, &eval_result))
        {
                if (eval_result.kind == TERK_FLOATING)
                        c_error_array_index_in_initializer_not_of_integer_type(self->logger, index);
                else
                        c_error_nonconstant_array_index_in_initializer(self->logger, index);
                return false;
        }

        uint index_val = avalue_get_u32(&eval_result.value);
        if (tree_array_is(parent->type, TAK_CONSTANT))
        {
                if (index_val >= tree_get_array_size(parent->type))
                {
                        c_error_array_index_in_initializer_exceeds_array_bounds(self->logger, index);
                        return false;
                }
        }

        c_initialized_object_set_index(parent, index_val);
        return c_initialized_object_new(self->ccontext,
                c_initialized_object_get_subobject(parent), true, parent);
}

static c_initialized_object* c_sema_check_designation(
        c_sema* self, c_initializer* it, c_initialized_object* parent)
{
        assert(it->pos != it->end);
        tree_expr* designation = *it->pos;
        assert(tree_expr_is(designation, TEK_DESIGNATION));

        c_initialized_object* result = parent;
        TREE_FOREACH_DESIGNATION_DESIGNATOR(designation, pdesignator, end)
        {
                result = tree_designator_is_field(*pdesignator)
                        ? c_sema_check_field_designator(self, *pdesignator, result)
                        : c_sema_check_array_designator(self, *pdesignator, result);
                if (!result)
                        return NULL;
        }

        if (!c_sema_check_initializer(self, result->type, tree_get_designation_init(designation)))
                return NULL;

        it->pos++;
        c_initialized_object_set_initialized(result);
        return result;
}

static void c_sema_set_incomplete_array_size(c_sema* self, tree_type* array, uint size)
{
        assert(tree_array_is(array, TAK_INCOMPLETE));
        int_value size_value;
        int_init(&size_value, 
                8 * tree_get_sizeof(self->target, c_sema_get_size_t_type(self)), false, size);
        tree_init_constant_array_type(array, tree_get_array_eltype(array), NULL, &size_value);
}

static bool _c_sema_check_initializer(c_sema*, tree_type*, c_initializer*, c_initialized_object*);

typedef enum
{
        CIK_INVALID,
        CIK_STRING,
        CIK_ORDINARY,
} c_initializer_kind;

// handles special cases like:
//      char a[] = "abc";
//      char a[] = { "abc" };
static c_initializer_kind c_sema_maybe_handle_special_array_initializer(
        c_sema* self, c_initialized_object* array, c_initializer* init)
{
        bool is_constant_array = tree_array_is(array->type, TAK_CONSTANT);
        tree_type* eltype = tree_get_array_eltype(array->type);
        if (!tree_type_is_scalar(eltype))
                return CIK_ORDINARY;

        bool in_init_list = false;
        tree_expr* expr = tree_desugar_expr(*init->expr);
        if (tree_expr_is(expr, TEK_INIT_LIST))
        {
                expr = tree_desugar_expr(*init->pos);
                in_init_list = true;
        }

        if (!tree_expr_is(expr, TEK_STRING_LITERAL))
                return CIK_ORDINARY;

        if (!tree_builtin_type_is(eltype, TBTK_UINT8) 
                && !tree_builtin_type_is(eltype, TBTK_INT8))
        {
                // todo
                c_error_wide_character_array_initialized_from_non_wide_string(self->logger, expr);
                return CIK_INVALID;
        }

        strentry entry;
        tree_get_id_strentry(self->context, tree_get_string_literal(expr), &entry);

        uint num_elems = (uint)(entry.size - 1); // ignore '\0' at first...
        if ((in_init_list && init->end - init->pos > 1) || !is_constant_array)
        {
                // ...except this cases:
                //      char a[3] = { "ab", 'c' };
                //      char a[] = "abc";
                num_elems += 1;
        }
      
        if (is_constant_array && tree_get_array_size(array->type) < num_elems)
        {
                c_error_initializer_string_is_too_long(self->logger, expr);
                return CIK_INVALID;
        }

        init->pos++;
        if (!is_constant_array)
                c_sema_set_incomplete_array_size(self, array->type, num_elems);

        c_initialized_object_set_index(array, num_elems);
        return CIK_STRING;
}

static bool _c_sema_check_array_or_record_initializer(
        c_sema* self, c_initialized_object* object, c_initializer* init)
{
        bool top_level = object->semantical_parent == NULL;
        c_initializer_kind init_kind = CIK_ORDINARY;
        if (object->kind == CIOK_ARRAY)
                init_kind = c_sema_maybe_handle_special_array_initializer(self, object, init);
        else
                ; // handle other cases...

        if (init_kind == CIK_INVALID)
                return false;

        if (init_kind == CIK_ORDINARY && top_level && !tree_expr_is(*init->expr, TEK_INIT_LIST))
        {
                c_error_invalid_initializer(self->logger, *init->expr);
                return false;
        }

        while (init->pos != init->end)
        {
                if (tree_expr_is(*init->pos, TEK_DESIGNATION))
                {
                        if (!top_level)
                                return true;
                        c_initialized_object* designation_parent = object->implicit
                                ? object->syntactical_parent : object;
                        if (!(object = c_sema_check_designation(self, init, designation_parent)))
                                return false;
                        while (!c_initialized_object_valid(object) && object->semantical_parent)
                        {
                                object = object->semantical_parent;
                                c_initialized_object_next_subobject(object);
                        }
                }
                else
                {
                        if (!c_initialized_object_valid(object))
                        {
                                if (!top_level)
                                        return true;
                                c_error_too_many_initializer_values(self->logger, tree_get_expr_loc(*init->pos));
                                return false;
                        }

                        if (!_c_sema_check_initializer(self,
                                c_initialized_object_get_subobject(object), init, object))
                        {
                                return false;
                        }
                        c_initialized_object_next_subobject(object);
                }
        }
        return true;
}

static bool c_sema_check_array_or_record_initializer(
        c_sema* self, tree_type* type, c_initializer* init, c_initialized_object* parent)
{
        c_initialized_object* object = c_initialized_object_new(self->ccontext, type, parent != NULL, parent);
        if (!_c_sema_check_array_or_record_initializer(self, object, init))
                return false;

        if (tree_type_is(object->type, TTK_ARRAY) && tree_array_is(object->type, TAK_INCOMPLETE))
                c_sema_set_incomplete_array_size(self, type, object->array.size);

        return true;
}

static bool c_sema_check_scalar_initializer(
        c_sema* self, tree_type* type, c_initializer* init, bool top_level)
{
        assert(tree_type_is_scalar(type));
        if (top_level && tree_expr_is(*init->expr, TEK_INIT_LIST))
        {
                if (tree_get_init_list_exprs_size(*init->expr) > 1)
                {
                        c_error_too_many_initializer_values(self->logger,
                                tree_get_expr_loc(tree_get_init_list_expr(*init->expr, 1)));
                        return false;
                }
        }

        tree_expr** expr = init->pos;
        if (tree_expr_is(*expr, TEK_INIT_LIST))
        {
                c_error_braces_around_scalar_initializer(self->logger, tree_get_expr_loc(*expr));
                return false;
        }
        else if (tree_expr_is(*expr, TEK_DESIGNATION))
        { 
                c_initialized_object object;
                c_initialized_object_init(&object, type, false, NULL);
                c_sema_check_designation(self, init, &object);
                return false;
        }

        c_assignment_conversion_result r;
        if (!c_sema_assignment_conversion(self, type, expr, &r))
        {
                c_error_invalid_scalar_initialization(self->logger, *expr, &r);
                return false;
        }

        tree_eval_result eval_result;
        if (c_sema_at_file_scope(self) && !tree_eval_expr(self->context, *expr, &eval_result))
        {
                c_error_initializer_element_isnt_constant(self->logger, *expr);
                return false;
        }

        return true;
}

static bool _c_sema_check_initializer(
        c_sema* self, tree_type* type, c_initializer* init, c_initialized_object* parent)
{
        assert(init->pos != init->end);
        bool top_level = parent == NULL;
        if (tree_type_is_array(type) || tree_type_is_record(type))
        {
                if (!top_level && tree_expr_is(*init->pos, TEK_INIT_LIST))
                {
                        if (!c_sema_check_initializer(self, type, *init->pos))
                                return false;
                }
                else
                        return c_sema_check_array_or_record_initializer(self, type, init, parent);
        }
        else if (!c_sema_check_scalar_initializer(self, type, init, top_level))
                return false;

        init->pos++;
        return true;
}

extern tree_expr* c_sema_check_initializer(c_sema* self, tree_type* type, tree_expr* init)
{
        // todo: free c_initialized_object pointers
        c_initializer i;
        c_initializer_init(&i, &init);
        return _c_sema_check_initializer(self, type, &i, NULL) ? init : NULL;
}

extern tree_expr* c_sema_new_designation(c_sema* self)
{
        return tree_new_designation(self->context, NULL);
}

extern tree_expr* c_sema_add_designation_designator(
        c_sema* self, tree_expr* designation, tree_designator* designator)
{
        tree_add_designation_designator(designation, self->context, designator);
        return designation;
}

extern tree_expr* c_sema_set_designation_initializer(c_sema* self, tree_expr* designation, tree_expr* init)
{
        assert(designation);
        tree_set_designation_init(designation, init);
        return designation;
}

extern tree_designator* c_sema_new_field_designator(c_sema* self, tree_location loc, tree_id field)
{
        return tree_new_field_designator(self->context, loc, field);
}

extern tree_designator* c_sema_new_array_designator(c_sema* self, tree_location loc, tree_expr* index)
{
        return tree_new_array_designator(self->context, loc, index);
}
