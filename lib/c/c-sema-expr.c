#include "scc/c/c-sema-expr.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-sema-conv.h"
#include "scc/c/c-info.h"
#include "scc/c/c-errors.h"

extern bool csema_require_object_pointer_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_object_pointer(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_pointer_to_object_type(self->logger, l);
                return false;
        }
        return true;
}
extern bool csema_require_function_pointer_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_function_pointer(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_pointer_to_function_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_integral_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_integer(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_integral_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_integer_expr(const csema* self, const tree_expr* e)
{
        return csema_require_integral_expr_type(self,
                tree_get_expr_type(e), tree_get_expr_loc(e));
}

extern bool csema_require_real_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_real(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_real_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_record_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_record(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_record_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_array_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_array(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_array_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_scalar_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_scalar(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_scalar_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_scalar_expr(const csema* self, const tree_expr* e)
{
        return csema_require_scalar_expr_type(self,
                tree_get_expr_type(e), tree_get_expr_loc(e));
}

extern bool csema_require_arithmetic_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_arithmetic(tree_desugar_ctype(t)))
        {
                cerror_expr_must_have_arithmetic_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_real_or_object_pointer_expr_type(
        const csema* self, const tree_type* t, tree_location l)
{
        t = tree_desugar_ctype(t);
        if (!tree_type_is_real(t) && !tree_type_is_pointer(t))
        {
                cerror_expr_must_have_real_or_pointer_to_object_type(self->logger, l);
                return false;
        }
        return true;
}

extern bool csema_require_lvalue_or_function_designator(
        const csema* self, const tree_expr* e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(e));
        if (!tree_expr_is_lvalue(e) && tree_get_type_kind(t) != TTK_FUNCTION)
        {
                cerror_expr_must_be_lvalue_or_function_designator(self->logger, e);
                return false;
        }
        return true;
}

extern bool csema_require_modifiable_lvalue(const csema* self, const tree_expr* e)
{
        if (!tree_expr_is_modifiable_lvalue(e))
        {
                cerror_expr_must_be_modifiable_lvalue(self->logger, e);
                return false;
        }
        return true;
}

extern bool csema_require_compatible_expr_types(
        const csema* self, const tree_type* a, const tree_type* b, tree_location l)
{
        if (!tree_types_are_same(a, b))
        {
                cerror_types_are_not_compatible(self->logger, l);
                return false;
        }
        return true;
}

extern tree_expr* csema_new_paren_expr(
        csema* self, tree_location lbracket_loc, tree_expr* expr, tree_location rbracket_loc)
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

extern tree_expr* csema_new_decl_expr(csema* self, tree_id id, tree_location id_loc)
{
        tree_decl* d = csema_require_local_decl(self, id_loc, TDK_UNKNOWN, id);
        if (!d)
                return NULL; // unknown decl

        tree_decl_kind dk = tree_get_decl_kind(d);
        tree_value_kind vk = TVK_LVALUE;
        tree_type* t = tree_desugar_type(tree_get_decl_type(d));

        if (dk == TDK_ENUMERATOR || dk == TDK_FUNCTION)
                vk = TVK_RVALUE;
        else if (dk != TDK_VAR)
                return NULL; // unknown decl kind

        return tree_new_decl_expr(self->context, vk, t, id_loc, d);
}

extern tree_expr* csema_new_sp_floating_literal(csema* self, tree_location loc, float v)
{
        float_value value;
        float_init_sp(&value, v);

        return tree_new_floating_literal(self->context,
                csema_get_float_type(self), loc, &value);
}

extern tree_expr* csema_new_dp_floating_literal(csema* self, tree_location loc, ldouble v)
{
        float_value value;
        float_init_dp(&value, v);

        return tree_new_floating_literal(self->context,
                csema_get_double_type(self), loc, &value);
}

extern tree_expr* csema_new_character_literal(csema* self, tree_location loc, int c)
{
        return tree_new_character_literal(self->context, csema_get_char_type(self), loc, c);
}

extern tree_expr* csema_new_integer_literal(
        csema* self, tree_location loc, suint64 v, bool signed_, bool ext)
{
        return tree_new_integer_literal(self->context,
                csema_get_int_type(self, signed_, ext), loc, v);
}

extern tree_expr* csema_new_string_literal(csema* self, tree_location loc, tree_id ref)
{
        return tree_new_string_literal(self->context, TVK_LVALUE,
                csema_get_type_for_string_literal(self, ref), loc, ref);
}

// c99 6.5.2.1 array subscripting
// One of the expressions shall have type "pointer to object type",
// the other expression shall have integer type, and the result has type "type".
extern tree_expr* csema_new_subscript_expr(
        csema* self, tree_location loc, tree_expr* lhs, tree_expr* rhs)
{
        if (!lhs || !rhs)
                return NULL;

        csema_unary_conversion(self, &lhs);
        tree_type* rt = csema_unary_conversion(self, &rhs);

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
                cerror_subscripted_value_isnt_array(self->logger, loc);
                return NULL;
        }
        if (!csema_require_integer_expr(self, index))
                return NULL;

        return tree_new_subscript_expr(self->context,
                TVK_LVALUE, tree_get_pointer_target(base_type), loc, lhs, rhs);
}

static bool csema_check_call_argument(
        csema* self, tree_type* arg_type, tree_expr** arg, uint pos)
{
        cassign_conv_result r;
        if (csema_assignment_conversion(self, arg_type, arg, &r))
                return true;

        tree_location loc = tree_get_expr_loc(*arg);
        switch (r.kind)
        {
                case CACRK_RHS_NOT_AN_ARITHMETIC:
                case CACRK_RHS_NOT_A_RECORD:
                case CACRK_INCOMPATIBLE_RECORDS:
                case CACRK_INCOMPATIBLE:
                        cerror_incompatible_type_for_argument(self->logger, loc, pos);
                        break;
                case CACRK_QUAL_DISCARTION:
                        cerror_passing_argument_discards_qualifer(
                                self->logger, loc, pos, r.discarded_quals);
                        break;
                case CACRK_INCOMPATIBLE_POINTERS:
                        cerror_passing_argument_from_incompatible_pointer_type(
                                self->logger, loc, pos);
                        break;
                default:
                        S_UNREACHABLE();
        }
        return false;
}

// c99 6.5.2.2 function calls
// The expression that denotes the called function shall have type pointer to function
// returning void or returning an object type other than an array type.
// If the expression that denotes the called function has a type that includes a prototype,
// the number of arguments shall agree with the number of parameters. Each argument shall
// have a type such that its value may be assigned to an object with the unqualified version
// of the type of its corresponding parameter.
extern tree_expr* csema_new_call_expr(
        csema* self, tree_location loc, tree_expr* lhs, dseq* args)
{
        if (!lhs || !args)
                return NULL;

        tree_type* t = csema_unary_conversion(self, &lhs);
        if (!csema_require_function_pointer_expr_type(self, t, tree_get_expr_loc(lhs)))
                return NULL;

        t = tree_desugar_type(tree_get_pointer_target(tree_desugar_ctype(t)));
        ssize nparams = tree_get_function_type_nparams(t);
        ssize nargs = dseq_size(args);
        if (nargs < nparams)
        {
                cerror_to_few_arguments(self->logger, lhs);
                return NULL;
        }
        if (!tree_function_type_is_vararg(t) && nargs > nparams)
        {
                cerror_to_many_arguments(self->logger, lhs);
                return NULL;
        }

        ssize i = 0;
        for (tree_type** atype = tree_get_function_type_params_begin(t); i < nparams; i++, atype++)
                if (!csema_check_call_argument(self, *atype, dseq_get(args, i), (uint)i + 1))
                        return NULL;

        for (; i < nargs; i++)
                csema_default_argument_promotion(self, dseq_get(args, i));

        tree_expr* call = tree_new_call_expr(self->context, TVK_RVALUE,
                tree_get_function_type_result(t), loc, lhs);
        tree_set_call_args(call, args);
        return call;
}

// c99 6.5.2.3 structure and union members
// The first operand of the . operator shall have a qualified or unqualified
// structure or union type, and the second operand shall name a member of that type.
// The first operand of the -> operator shall have type "pointer to qualified 
// or unqualified structure" or "pointer to qualified or unqualified union",
// and the second operand shall name a member of the type pointed to.
extern tree_expr* csema_new_member_expr(
        csema* self,
        tree_location loc,
        tree_expr* lhs,
        tree_id id,
        tree_location id_loc,
        bool is_arrow)
{
        if (!lhs)
                return NULL;

        tree_location lhs_loc = tree_get_expr_loc(lhs);
        tree_type* t = csema_array_function_to_pointer_conversion(self, &lhs);

        if (is_arrow)
        {
                t = csema_lvalue_conversion(self, &lhs);
                if (!csema_require_object_pointer_expr_type(self, t, lhs_loc))
                        return NULL;

                t = tree_get_pointer_target(t);
        }
        t = tree_desugar_type(t);
        if (!csema_require_record_expr_type(self, t, lhs_loc))
                return NULL;
        
        tree_decl* record = tree_get_decl_type_entity(t);
        tree_decl* m = csema_require_member_decl(self, id_loc, record, id);
        if (!m)
                return NULL;

        return tree_new_member_expr(self->context, 
                TVK_LVALUE, tree_get_decl_type(m), loc, lhs, m, is_arrow);
}

// c99 6.5.2.4/6.5.3.1
// The operand of the postfix (or prefix) increment or decrement operator shall have
// qualified or unqualified real or pointer type and shall be a modifiable lvalue.
static tree_type* csema_check_inc_dec_expr(csema* self, tree_expr** expr)
{
        tree_type* t = csema_array_function_to_pointer_conversion(self, expr);
        if (!csema_require_real_or_object_pointer_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;
        if (!csema_require_modifiable_lvalue(self, *expr))
                return NULL;
        return t;
}

// 6.5.3.2 address and inderection operators
// The operand of the unary & operator shall be either a function designator, the result of a
// [] or unary * operator, or an lvalue that designates an object
// that is not a bit - field and is not declared with the register storage - class specifier.
static tree_type* csema_check_address_expr(csema* self, tree_expr** expr)
{
        if (!csema_require_lvalue_or_function_designator(self, *expr))
                return NULL;

        return csema_new_pointer(self, TTQ_UNQUALIFIED, tree_get_expr_type(*expr));
}

// 6.5.3.2 address and inderection operators
// The operand of the unary * operator shall have pointer type.
static tree_type* csema_check_dereference_expr(csema* self, tree_expr** expr, tree_value_kind* vk)
{
        tree_type* t = tree_desugar_type(csema_unary_conversion(self, expr));
        if (!csema_require_object_pointer_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;
 
        t = tree_get_pointer_target(t);
        *vk = TVK_LVALUE;
        return t;
}

// 6.5.3.3 unary arithmetic
// The operand of the ~ operator shall have integer type
static tree_type* csema_check_log_not_expr(csema* self, tree_expr** expr)
{
        tree_type* t = csema_unary_conversion(self, expr);
        if (!csema_require_scalar_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return csema_get_logical_operation_type(self);
}

// 6.5.3.3 unary arithmetic
// The operand of the unary + or - operator shall have arithmetic type
static tree_type* csema_check_plus_minus_expr(csema* self, tree_expr** expr)
{
        tree_type* t = csema_unary_conversion(self, expr);
        if (!csema_require_arithmetic_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return csema_integer_promotion(self, expr);
}

// 6.5.3.3 unary arithmetic
// The operand of the ! operator shall have scalar type
static tree_type* csema_check_not_expr(csema* self, tree_expr** expr)
{
        tree_type* t = csema_unary_conversion(self, expr);
        if (!csema_require_integral_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return csema_integer_promotion(self, expr);
}

extern tree_expr* csema_new_unary_expr(
        csema* self, tree_location loc, tree_unop_kind opcode, tree_expr* expr)
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
                        t = csema_check_inc_dec_expr(self, &expr);
                        break;

                case TUK_ADDRESS:
                        t = csema_check_address_expr(self, &expr);
                        break;

                case TUK_DEREFERENCE:
                        t = csema_check_dereference_expr(self, &expr, &vk);
                        break;

                case TUK_LOG_NOT:
                        t = csema_check_log_not_expr(self, &expr);
                        break;

                case TUK_PLUS:
                case TUK_MINUS:
                        t = csema_check_plus_minus_expr(self, &expr);
                        break;

                case TUK_NOT:
                        t = csema_check_not_expr(self, &expr);
                        break;

                default:
                        // unknown unop
                        S_UNREACHABLE();
        }
        if (!t)
                return NULL;

        return tree_new_unop(self->context, vk, t, loc, opcode, expr);
}

// c99 6.5.3.4
// The sizeof operator shall not be applied to an expression that has function type or an
// incomplete type, to the parenthesized name of such a type, or to an expression that
// designates a bit - field member.
extern tree_expr* csema_new_sizeof_expr(csema* self, tree_location loc, csizeof_operand* op)
{
        if (!op)
                return NULL;

        tree_type* rt = op->unary ? tree_get_expr_type(op->expr) : op->type;
        if (tree_type_is(rt, TTK_FUNCTION))
        {
                cerror_operand_of_sizeof_is_function(self->logger, op);
                return NULL;
        }
        if (!csema_require_complete_type(self, op->loc, rt))
                return NULL;
        if (op->unary && tree_expr_designates_bitfield(op->expr))
        {
                cerror_operand_of_sizeof_is_bitfield(self->logger, op);
                return NULL;
        }

        return tree_new_sizeof_expr(self->context,
                csema_get_size_t_type(self), loc, op->pointer, op->unary);
}

// c99 6.5.4 cast operators
// Unless the type name specifies a void type, the type name shall specify qualified or
// unqualified scalar type and the operand shall have scalar type.
// Conversions that involve pointers, other than where permitted by the constraints of
// 6.5.16.1, shall be specified by means of an explicit cast.
extern tree_expr* csema_new_cast_expr(
        csema* self, tree_location loc, tree_type* type, tree_expr* expr)
{
        if (!type || !expr)
                return NULL;

        tree_type* et = tree_desugar_type(csema_unary_conversion(self, &expr));
        if (!tree_type_is_void(type))
        {
                if (!csema_require_scalar_expr_type(self, type, loc))
                        return NULL;
                if (!csema_require_scalar_expr_type(self, et, tree_get_expr_loc(expr)))
                        return NULL;
        }

        tree_value_kind vk = tree_get_expr_value_kind(expr);
        return tree_new_explicit_cast_expr(self->context, vk, loc, type, expr);
}

// 6.5.5 multiplicative
// Each of the operands shall have arithmetic type.
static tree_type* csema_check_mul_div_expr(
        csema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);

        if (!csema_require_arithmetic_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;
        if (!csema_require_arithmetic_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;

        return csema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
}

// 6.5.7/6.5.10-13
// Each of the operands shall have integer type.
static tree_type* csema_check_bitwise_expr(
        csema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);

        if (!csema_require_integral_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;
        if (!csema_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;

        return csema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
}

// 6.5.13/6.5.14 logical and/or operator
// Each of the operands shall have scalar type
static tree_type* csema_check_log_expr(
        csema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);

        if (!csema_require_scalar_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;
        if (!csema_require_scalar_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;

        return csema_get_logical_operation_type(self);
}

// 6.5.8 Relational operators
// One of the following shall hold:
// - both operands have real type;
// - both operands are pointers to qualified or unqualified versions of
// compatible object types; or
// - both operands are pointers to qualified or unqualified versions of
// compatible incomplete types
static tree_type* csema_check_relational_expr(
        csema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_real(lt) && tree_type_is_real(rt))
        {
                if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                        csema_usual_arithmetic_conversion(self, lhs, rhs, true);
        }
        else if (tree_type_is_object_pointer(lt) && tree_type_is_object_pointer(rt))
        {
                if (!csema_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
        }
        else
        {
                cerror_invalid_binop_operands(self->logger, loc, opcode);
                return NULL;
        }

        return csema_get_logical_operation_type(self);
}

// 6.5.9 Equality operators
// One of the following shall hold:
// - both operands have arithmetic type;
// - both operands are pointers to qualified or unqualified versions of compatible types;
// - one operand is a pointer to an object or incomplete type and the other is a pointer to a
// qualified or unqualified version of void; or
// - one operand is a pointer and the other is a null pointer constant.
static tree_type* csema_check_compare_expr(
        csema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                csema_usual_arithmetic_conversion(self, lhs, rhs, true);
        else if (tree_type_is_pointer(lt) && tree_expr_is_null_pointer_constant(*rhs))
                ;
        else if (tree_type_is_pointer(rt) && tree_expr_is_null_pointer_constant(*lhs))
                ;
        else if (tree_type_is_pointer(lt) && tree_type_is_pointer(rt))
        {
                tree_type* ltarget = tree_desugar_type(tree_get_pointer_target(lt));
                tree_type* rtarget = tree_desugar_type(tree_get_pointer_target(rt));

                if ((tree_type_is_incomplete(ltarget) && !tree_type_is_void(rtarget))
                 || (tree_type_is_incomplete(rtarget) && !tree_type_is_void(ltarget)))
                {
                        cerror_cmp_of_distinct_pointers(self->logger, loc);
                        return NULL;
                }
                else if (!csema_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
        }
        else
        {
                cerror_invalid_binop_operands(self->logger, loc, opcode);
                return NULL;
        }
        return csema_get_logical_operation_type(self);
}

// 6.5.5 multiplicative
// The operands of the % operator shall have integer type
static tree_type* csema_check_mod_expr(
        csema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);

        if (!csema_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;
        if (!csema_require_integral_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;

        return csema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
}

// 6.5.6 additive
// For addition, either both operands shall have arithmetic type, or one operand shall be a
// pointer to an object type and the other shall have integer type.
// (Incrementing is equivalent to adding 1.)
static tree_type* csema_check_add_expr(
        csema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return csema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
        else if (tree_type_is_object_pointer(lt))
        {
                if (!csema_require_integral_expr_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        else if (tree_type_is_object_pointer(rt))
        {
                if (!csema_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                        return NULL;

                return rt;
        }
        cerror_invalid_binop_operands(self->logger, loc, is_assign ? TBK_ADD_ASSIGN : TBK_ADD);
        return NULL;
}

// 6.5.6 additive
// For subtraction, one of the following shall hold :
// - both operands have arithmetic type;
// - both operands are pointers to qualified or unqualified versions
// of compatible object types; or
// - the left operand is a pointer to an object type and the right operand has integer type.
// (Decrementing is equivalent to subtracting 1.)
static tree_type* csema_check_sub_expr(
        csema* self, tree_expr** lhs, tree_expr** rhs, tree_location loc, bool is_assign)
{
        tree_type* lt = is_assign
                ? tree_get_expr_type(*lhs)
                : csema_unary_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return csema_usual_arithmetic_conversion(self, lhs, rhs, !is_assign);
        else if (tree_type_is_object_pointer(lt))
        {
                if (tree_type_is_object_pointer(rt) && tree_types_are_same(lt, rt))
                        return lt;

                if (!csema_require_integral_expr_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        cerror_invalid_binop_operands(self->logger, loc, is_assign ? TBK_SUB_ASSIGN : TBK_SUB);
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
static tree_type* csema_check_assign_expr(
        csema* self,
        tree_expr** lhs,
        tree_expr** rhs,
        tree_location loc,
        tree_type* compound_type)
{
        tree_type* lt = csema_array_function_to_pointer_conversion(self, lhs);
        if (!csema_require_modifiable_lvalue(self, *lhs))
                return NULL;

        cassign_conv_result r;
        tree_type* t = csema_assignment_conversion(
                self, compound_type ? compound_type : lt, rhs, &r);
        if (t)
                return t;

        tree_type* rt = tree_get_expr_type(*rhs);
        tree_location rloc = tree_get_expr_loc(*rhs);

        if (r.kind == CACRK_INCOMPATIBLE)
                cerror_invalid_binop_operands(self->logger, loc, TBK_ASSIGN);
        else if (r.kind == CACRK_RHS_NOT_AN_ARITHMETIC)
                csema_require_arithmetic_expr_type(self, rt, rloc);
        else if (r.kind == CACRK_RHS_NOT_A_RECORD)
                csema_require_record_expr_type(self, rt, rloc);
        else if (r.kind == CACRK_INCOMPATIBLE_RECORDS)
                csema_require_compatible_expr_types(self, lt, rt, loc);
        else if (r.kind == CACRK_QUAL_DISCARTION)
                cerror_assignment_discards_quals(self->logger, loc, r.discarded_quals);
        else if (r.kind == CACRK_INCOMPATIBLE_POINTERS)
                cerror_assignment_from_incompatible_pointer_type(self->logger, loc);
        return NULL;
}

// 6.5.16.2 Compound assignment
// For the operators += and -= only, either the left operand shall be a pointer to an object
// type and the right shall have integer type, or the left operand shall have qualified or
// unqualified arithmetic type and the right shall have arithmetic type.
static tree_type* csema_check_add_sub_assign_expr(
        csema* self, tree_binop_kind opcode, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        tree_type* lt = csema_array_function_to_pointer_conversion(self, lhs);
        tree_type* rt = csema_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (!csema_require_modifiable_lvalue(self, *lhs))
                return NULL;

        if (tree_type_is_object_pointer(lt))
        {
                if (!csema_require_integral_expr_type(self, rt, rl))
                        return NULL;
        }
        else if (tree_type_is_arithmetic(lt))
        {
                if (!csema_require_arithmetic_expr_type(self, rt, rl))
                        return NULL;
        }
        else
        {
                cerror_invalid_binop_operands(self->logger, loc, opcode);
                return NULL;
        }

        return lt;
}

// returns type of the binary operator
static inline tree_type* csema_check_binop(
        csema* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        if (!lhs || !rhs)
                return NULL;
        
        tree_type* compound;
        switch (opcode)
        {
                case TBK_MUL:
                case TBK_DIV:
                        return csema_check_mul_div_expr(self, lhs, rhs, loc, false);
                case TBK_MOD:
                        return csema_check_mod_expr(self, lhs, rhs, loc, false);
                case TBK_ADD:
                        return csema_check_add_expr(self, lhs, rhs, loc, false);
                case TBK_SUB:
                        return csema_check_sub_expr(self, lhs, rhs, loc, false);
                case TBK_SHL:
                case TBK_SHR:
                case TBK_OR:
                case TBK_AND:
                case TBK_XOR:
                        return csema_check_bitwise_expr(self, lhs, rhs, loc, false);
                case TBK_LE:
                case TBK_LEQ:
                case TBK_GR:
                case TBK_GEQ:
                        return csema_check_relational_expr(self, opcode, loc, lhs, rhs);
                case TBK_EQ:
                case TBK_NEQ:
                        return csema_check_compare_expr(self, opcode, loc, lhs, rhs);
                case TBK_LOG_AND:
                case TBK_LOG_OR:
                        return csema_check_log_expr(self, opcode, loc, lhs, rhs);
                case TBK_ASSIGN:
                        return csema_check_assign_expr(self, lhs, rhs, loc, NULL);
                case TBK_ADD_ASSIGN:
                        if (!(csema_check_add_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return csema_check_add_sub_assign_expr(self, opcode, lhs, rhs, loc);
                case TBK_SUB_ASSIGN:
                        if (!(csema_check_sub_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return csema_check_add_sub_assign_expr(self, opcode, lhs, rhs, loc);
                case TBK_MUL_ASSIGN:
                case TBK_DIV_ASSIGN:
                        if (!(compound = csema_check_mul_div_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return csema_check_assign_expr(self, lhs, rhs, loc, compound);
                case TBK_MOD_ASSIGN:
                        if (!(compound = csema_check_mod_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return csema_check_assign_expr(self, lhs, rhs, loc, compound);
                case TBK_SHL_ASSIGN:
                case TBK_SHR_ASSIGN:
                case TBK_AND_ASSIGN:
                case TBK_OR_ASSIGN:
                case TBK_XOR_ASSIGN:
                        if (!(compound = csema_check_bitwise_expr(self, lhs, rhs, loc, true)))
                                return NULL;
                        return csema_check_assign_expr(self, lhs, rhs, loc, compound);
                case TBK_COMMA:
                        csema_unary_conversion(self, lhs);
                        return csema_unary_conversion(self, rhs);
                default:
                        S_ASSERT(0 && "Invalid binop kind");
                        return NULL;

        }
}

extern tree_expr* csema_new_binary_expr(
        csema* self, tree_location loc, tree_binop_kind opcode, tree_expr* lhs, tree_expr* rhs)
{
        tree_type* t = csema_check_binop(self, opcode, loc, &lhs, &rhs);
        if (!t)
                return NULL;

        return tree_new_binop(self->context, TVK_RVALUE, t, loc, opcode, lhs, rhs);
}

static tree_type* csema_check_conditional_operator_pointer_types(
        csema* self, tree_expr* lhs, tree_expr* rhs, tree_location loc)
{
        tree_type* lt = tree_get_expr_type(lhs);
        tree_type* rt = tree_get_expr_type(rhs);
        bool lpointer = tree_type_is_object_pointer(lt);
        bool rpointer = tree_type_is_object_pointer(rt);

        tree_type* target = NULL;
        tree_type_quals quals = TTQ_UNQUALIFIED;
        if (rpointer && tree_expr_is_null_pointer_constant(lhs))
        {
                target = tree_get_pointer_target(rt);
                quals = tree_get_type_quals(target);
                if (lpointer)
                        quals |= tree_get_type_quals(tree_get_pointer_target(lt));
        }
        else if (lpointer && tree_expr_is_null_pointer_constant(rhs))
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
                else if (tree_types_are_same(tree_get_unqualified_type(ltarget),
                                             tree_get_unqualified_type(rtarget)))
                {
                        target = ltarget;
                }
                else
                {
                        cerror_pointer_type_mismatch(self->logger, loc);
                        return NULL;
                }
                quals = tree_get_type_quals(ltarget) | tree_get_type_quals(rtarget);
        }
        else
                return NULL;

        target = tree_new_qual_type(self->context, quals, target);
        return csema_new_pointer(self, TTQ_UNQUALIFIED, target);
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
extern tree_expr* csema_new_conditional_expr(
        csema* self,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs)
{
        if (!condition || !lhs || !rhs)
                return NULL;

        tree_type* ct = csema_unary_conversion(self, &condition);
        if (!csema_require_scalar_expr_type(self, ct, tree_get_expr_loc(condition)))
                return NULL;

        tree_type* t = NULL;
        tree_type* lt = csema_unary_conversion(self, &lhs);
        tree_type* rt = csema_unary_conversion(self, &rhs);
        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                t = csema_usual_arithmetic_conversion(self, &lhs, &rhs, true);
        else if (tree_type_is_record(lt) && tree_type_is_record(rt))
        {
                if (!csema_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
                t = lt;
        }
        else if (tree_type_is_void(lt) && tree_type_is_void(rt))
                t = lt;
        else if ((t = csema_check_conditional_operator_pointer_types(self, lhs, rhs, loc)))
                ;
        else
        {
                cerror_type_mismatch(self->logger, loc);
                return NULL;
        }

        S_ASSERT(t);
        return tree_new_conditional_expr(self->context, TVK_RVALUE, t, loc, condition, lhs, rhs);
}

extern tree_designation* csema_new_designation(csema* self)
{
        return tree_new_designation(self->context, NULL);
}

extern tree_designation* csema_finish_designation(
        csema* self, tree_expr* list, tree_designation* d, tree_expr* init)
{
        tree_set_designation_initializer(d, init);
        tree_add_init_designation(list, d);
        return d;
}

extern bool csema_add_empty_designation(csema* self, tree_expr* initializer_list)
{
        return csema_finish_designation(self, initializer_list, csema_new_designation(self), NULL);
}

extern tree_type* csema_get_designation_type(csema* self, tree_designation* d)
{
        return csema_get_designator_type(self, tree_get_designation_last(d));
}

extern tree_designator* csema_new_member_designator(
        csema* self, tree_location loc, tree_type* t, tree_id name)
{
        t = tree_desugar_type(t);
        if (!csema_require_record_expr_type(self, t, loc))
                return NULL;

        tree_decl* record = tree_get_decl_type_entity(t);
        tree_decl* m = csema_require_member_decl(self, loc, record, name);
        if (!m)
                return NULL;
        
        return tree_new_member_designator(self->context, m);
}

extern tree_designator* csema_new_array_designator(
        csema* self, tree_location loc, tree_type* t, tree_expr* index)
{
        t = tree_desugar_type(t);
        if (!csema_require_array_expr_type(self, t, loc))
                return NULL;

        tree_type* eltype = tree_get_array_eltype(t);
        if (!eltype)
                return NULL;

        return tree_new_array_designator(self->context, eltype, index);
}

extern tree_designator* csema_finish_designator(
        csema* self, tree_designation* designation, tree_designator* designator)
{
        tree_add_designation_designator(designation, designator);
        return designator;
}

extern tree_type* csema_get_designator_type(csema* self, tree_designator* d)
{
        if (!d)
                return NULL;

        if (tree_designator_is(d, TDK_DES_ARRAY))
                return tree_get_array_designator_type(d);

        tree_decl* member = tree_get_member_designator_decl(d);
        return tree_get_decl_type(member);
}

extern tree_expr* csema_new_init_expr(csema* self, tree_location loc)
{
        return tree_new_init_expr(self->context, loc);
}

extern tree_expr* csema_finish_expr(csema* self, tree_expr* expr)
{
        if (!expr)
                return NULL;

        csema_unary_conversion(self, &expr);
        return expr;
}