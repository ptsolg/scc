#include "c-prog-exp.h"
#include "c-prog-type.h"
#include "c-prog-conversions.h"
#include "c-info.h"

extern bool cprog_require_object_pointer_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_object_pointer(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have pointer-to-object type");
                return false;
        }
        return true;
}
extern bool cprog_require_function_pointer_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_function_pointer(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have pointer-to-function type");
                return false;
        }
        return true;
}

extern bool cprog_require_integral_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_integer(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have integral type");
                return false;
        }
        return true;
}

extern bool cprog_require_real_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_real(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have real type");
                return false;
        }
        return true;
}

extern bool cprog_require_record_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_record(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have struct or union type");
                return false;
        }
        return true;
}

extern bool cprog_require_array_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_array(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have array type");
                return false;
        }
        return true;
}

extern bool cprog_require_scalar_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_scalar(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have scalar type");
                return false;
        }
        return true;
}

extern bool cprog_require_arithmetic_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        if (!tree_type_is_arithmetic(tree_desugar_ctype(t)))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have arithmetic type");
                return false;
        }
        return true;
}

extern bool cprog_require_real_or_object_pointer_expr_type(
        const cprog* self, const tree_type* t, tree_location l)
{
        t = tree_desugar_ctype(t);
        if (!tree_type_is_real(t) && !tree_type_is_pointer(t))
        {
                cerror(self->error_manager, CES_ERROR, l,
                        "expression must have real or pointer-to-object type");
                return false;
        }
        return true;
}

extern bool cprog_require_lvalue_or_function_designator(
        const cprog* self, const tree_expr* e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(e));
        if (!tree_expr_is_lvalue(e) && tree_get_type_kind(t) != TTK_FUNCTION)
        {
                cerror(self->error_manager, CES_ERROR, tree_get_expr_loc(e),
                        "expression must be an lvalue or function designator");
                return false;
        }
        return true;
}

extern bool cprog_require_modifiable_lvalue(const cprog* self, const tree_expr* e)
{
        if (!tree_expr_is_modifiable_lvalue(e))
        {
                cerror(self->error_manager, CES_ERROR, tree_get_expr_loc(e),
                        "expression must be a modifiable lvalue");
                return false;
        }
        return true;
}

extern bool cprog_require_compatible_expr_types(
        const cprog* self, const tree_type* a, const tree_type* b, tree_location l)
{
        if (!tree_types_are_same(a, b))
        {
                // todo: warning?
                cerror(self->error_manager, CES_ERROR, l, "types are not compatible");
                return false;
        }
        return true;
}

extern tree_expr* cprog_build_paren_expr(cprog* self, tree_location loc, tree_expr* expr)
{
        if (!expr)
                return NULL;

        return tree_new_paren_expr(
                self->context,
                tree_get_expr_value_kind(expr),
                tree_get_expr_type(expr),
                loc,
                expr);
}

extern tree_expr* cprog_build_decl_expr(cprog* self, tree_location loc, tree_id name)
{
        tree_decl* d = cprog_require_local_decl(self, loc, TDK_UNKNOWN, name);
        if (!d)
                return NULL; // unknown decl

        tree_decl_kind dk = tree_get_decl_kind(d);
        tree_value_kind vk = TVK_LVALUE;
        tree_type* t = tree_desugar_type(tree_get_decl_type(d));

        if (dk == TDK_ENUMERATOR || dk == TDK_FUNCTION)
                vk = TVK_RVALUE;
        else if (dk != TDK_VAR)
                return NULL; // unknown decl kind

        return tree_new_decl_expr(self->context, vk, t, loc, d);
}

extern tree_expr* cprog_build_floating_literal(cprog* self, tree_location loc, float v)
{
        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_FLOAT);
        return tree_new_floating_literal(self->context, t, loc, v);
}

extern tree_expr* cprog_build_floating_lliteral(cprog* self, tree_location loc, ldouble v)
{
        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_DOUBLE);
        return tree_new_floating_lliteral(self->context, t, loc, v);
}

extern tree_expr* cprog_build_character_literal(cprog* self, tree_location loc, int c)
{
        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT8);
        return tree_new_character_literal(self->context, t, loc, c);
}

extern tree_expr* cprog_build_integer_literal(
        cprog* self, tree_location loc, suint64 v, bool signed_, bool ext)
{
        tree_builtin_type_kind btk = ext ? TBTK_INT64 : TBTK_INT32;
        if (!signed_)
                btk = ext ? TBTK_UINT64 : TBTK_UINT32;

        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, btk);
        return tree_new_integer_literal(self->context, t, loc, v);
}

extern tree_expr* cprog_build_string_literal(cprog* self, tree_location loc, tree_id ref)
{
        tree_type* t = cprog_build_pointer(self, TTQ_UNQUALIFIED,
                cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT8));
        return tree_new_string_literal(self->context, t, loc, ref);
}

// c99 6.5.2.1 array subscripting
// One of the expressions shall have type "pointer to object type",
// the other expression shall have integer type, and the result has type "type".
extern tree_expr* cprog_build_subscript_expr(
        cprog* self, tree_location loc, tree_expr* lhs, tree_expr* rhs)
{
        if (!lhs || !rhs)
                return NULL;

        tree_type* lt = tree_desugar_type(cprog_perform_unary_conversion(self, &lhs));
        tree_type* rt = tree_desugar_type(tree_get_expr_type(rhs));
        tree_location rl = tree_get_expr_loc(rhs);
        tree_type* t = NULL;

        if (tree_type_is_pointer(lt))
        {
                if (!cprog_require_integral_expr_type(self, rt, rl))
                        return NULL;

                t = tree_get_pointer_target(lt);
        }
        else if (cprog_require_object_pointer_expr_type(self, rt, rl))
        {
                if (!cprog_require_integral_expr_type(self, lt, tree_get_expr_loc(lhs)))
                        return NULL;

                t = tree_get_pointer_target(rt);
        }
        else
                return NULL;

        return tree_new_subscript_expr(self->context, TVK_LVALUE, t, loc, lhs, rhs);
}

// c99 6.5.2.2 function calls
// The expression that denotes the called function shall have type pointer to function
// returning void or returning an object type other than an array type.
// If the expression that denotes the called function has a type that includes a prototype,
// the number of arguments shall agree with the number of parameters. Each argument shall
// have a type such that its value may be assigned to an object with the unqualified version
// of the type of its corresponding parameter.
extern tree_expr* cprog_build_call_expr(
        cprog* self, tree_location loc, tree_expr* lhs, objgroup* args)
{
        if (!lhs || !args)
                return NULL;

        tree_type* t = cprog_perform_unary_conversion(self, &lhs);
        if (!cprog_require_function_pointer_expr_type(self, t, tree_get_expr_loc(lhs)))
                return NULL;

        t = tree_get_pointer_target(tree_desugar_ctype(t));
        if (tree_get_function_type_nparams(t) != objgroup_size(args))
                return NULL;
  
        tree_expr* call = tree_new_call_expr(self->context, TVK_RVALUE,
                tree_get_function_restype(t), loc, lhs);

        tree_type** typeit = tree_get_function_type_begin(t);
        OBJGROUP_FOREACH(args, tree_expr**, it)
        {
                tree_expr* arg = cprog_build_impl_cast(self, *it, *typeit);
                tree_add_call_arg(call, arg);
                typeit++;
        }
        return call;
}

// c99 6.5.2.3 structure and union members
// The first operand of the . operator shall have a qualified or unqualified
// structure or union type, and the second operand shall name a member of that type.
// The first operand of the -> operator shall have type "pointer to qualified 
// or unqualified structure" or "pointer to qualified or unqualified union",
// and the second operand shall name a member of the type pointed to.
extern tree_expr* cprog_build_member_expr(
        cprog* self,
        tree_location loc,
        tree_expr* lhs,
        tree_id id,
        tree_location id_loc,
        bool is_arrow)
{
        if (!lhs)
                return NULL;

        tree_location lhs_loc = tree_get_expr_loc(lhs);
        tree_type* t = cprog_perform_array_function_to_pointer_conversion(self, &lhs);

        if (is_arrow)
        {
                t = cprog_perform_lvalue_conversion(self, &lhs);
                if (!cprog_require_object_pointer_expr_type(self, t, lhs_loc))
                        return NULL;

                t = tree_get_pointer_target(t);
        }
        t = tree_desugar_type(t);
        if (!cprog_require_record_expr_type(self, t, lhs_loc))
                return NULL;
        
        tree_decl* record = tree_get_decl_type_entity(t);
        tree_decl* m = cprog_require_member_decl(self, id_loc, record, id);
        if (!m)
                return NULL;

        return tree_new_member_expr(self->context, 
                TVK_LVALUE, tree_get_decl_type(m), loc, lhs, m, is_arrow);
}

// c99 6.5.2.4/6.5.3.1
// The operand of the postfix (or prefix) increment or decrement operator shall have
// qualified or unqualified real or pointer type and shall be a modifiable lvalue.
static tree_type* cprog_check_inc_dec_op(cprog* self, tree_expr** expr)
{
        tree_type* t = cprog_perform_array_function_to_pointer_conversion(self, expr);
        if (!cprog_require_real_or_object_pointer_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;
        if (!cprog_require_modifiable_lvalue(self, *expr))
                return NULL;
        return t;
}

// 6.5.3.2 address and inderection operators
// The operand of the unary & operator shall be either a function designator, the result of a
// [] or unary * operator, or an lvalue that designates an object
// that is not a bit - field and is not declared with the register storage - class specifier.
static tree_type* cprog_check_address_op(cprog* self, tree_expr** expr)
{
        if (!cprog_require_lvalue_or_function_designator(self, *expr))
                return NULL;

        return cprog_build_pointer(self, TTQ_UNQUALIFIED, tree_get_expr_type(*expr));
}

// 6.5.3.2 address and inderection operators
// The operand of the unary * operator shall have pointer type.
static tree_type* cprog_check_dereference_op(cprog* self, tree_expr** expr, tree_value_kind* vk)
{
        tree_type* t = tree_desugar_type(cprog_perform_unary_conversion(self, expr));
        tree_type_kind tk = tree_get_type_kind(t);

        if (!cprog_require_object_pointer_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;
 
        t = tree_get_pointer_target(t);
        *vk = TVK_LVALUE;
        return t;
}

// 6.5.3.3 unary arithmetic
// The operand of the ~ operator shall have integer type
static tree_type* cprog_check_log_not_op(cprog* self, tree_expr** expr)
{
        tree_type* t = cprog_perform_unary_conversion(self, expr);
        if (!cprog_require_scalar_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

// 6.5.3.3 unary arithmetic
// The operand of the unary + or - operator shall have arithmetic type
static tree_type* cprog_check_plus_minus_op(cprog* self, tree_expr** expr)
{
        tree_type* t = cprog_perform_unary_conversion(self, expr);
        if (!cprog_require_arithmetic_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return cprog_perform_integer_promotion(self, expr);
}

// 6.5.3.3 unary arithmetic
// The operand of the ! operator shall have scalar type
static tree_type* cprog_check_not_op(cprog* self, tree_expr** expr)
{
        tree_type* t = cprog_perform_unary_conversion(self, expr);
        if (!cprog_require_integral_expr_type(self, t, tree_get_expr_loc(*expr)))
                return NULL;

        return cprog_perform_integer_promotion(self, expr);
}

extern tree_expr* cprog_build_unop(
        cprog* self, tree_location loc, tree_unop_kind opcode, tree_expr* expr)
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
                        t = cprog_check_inc_dec_op(self, &expr);
                        break;

                case TUK_ADDRESS:
                        t = cprog_check_address_op(self, &expr);
                        break;

                case TUK_DEREFERENCE:
                        t = cprog_check_dereference_op(self, &expr, &vk);
                        break;

                case TUK_LOG_NOT:
                        t = cprog_check_log_not_op(self, &expr);
                        break;

                case TUK_PLUS:
                case TUK_MINUS:
                        t = cprog_check_plus_minus_op(self, &expr);
                        break;

                case TUK_NOT:
                        t = cprog_check_not_op(self, &expr);
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
extern tree_expr* cprog_build_sizeof(cprog* self, tree_location loc, csizeof_rhs* rhs)
{
        if (!rhs)
                return NULL;

        tree_type* rt = rhs->unary ? tree_get_expr_type(rhs->expr) : rhs->type;
        if (tree_type_is(rt, TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, rhs->loc,
                        "operand of sizeof may not be a function");
                return NULL;
        }
        if (!cprog_require_complete_type(self, rhs->loc, rt))
                return NULL;
        if (rhs->unary && tree_expr_designates_bitfield(rhs->expr))
        {
                cerror(self->error_manager, CES_ERROR, rhs->loc,
                        "operand of sizeof may not be a bitfield");
                return NULL;
        }

        tree_builtin_type_kind btk = TBTK_UINT32;
        if (tree_target_is(tree_get_module_target(self->module), TTARGET_X64))
                btk = TBTK_UINT64;

        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, btk);
        return tree_new_sizeof_expr(self->context, t, loc, rhs->pointer, rhs->unary);
}

// c99 6.5.4 cast operators
// Unless the type name specifies a void type, the type name shall specify qualified or
// unqualified scalar type and the operand shall have scalar type.
// Conversions that involve pointers, other than where permitted by the constraints of
// 6.5.16.1, shall be specified by means of an explicit cast.
extern tree_expr* cprog_build_cast(
        cprog* self, tree_location loc, tree_type* type, tree_expr* expr)
{
        if (!type || !expr)
                return NULL;

        tree_type* et = tree_desugar_type(cprog_perform_unary_conversion(self, &expr));
        if (!tree_type_is_void(type))
        {
                if (!cprog_require_scalar_expr_type(self, type, loc))
                        return NULL;
                if (!cprog_require_scalar_expr_type(self, et, tree_get_expr_loc(expr)))
                        return NULL;
        }

        tree_value_kind vk = tree_get_expr_value_kind(expr);
        return tree_new_exprlicit_cast_expr(self->context, vk, loc, type, expr);
}

// 6.5.5 multiplicative
// Each of the operands shall have arithmetic type.
static tree_type* cprog_check_mul_div_op(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_arithmetic_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;
        if (!cprog_require_arithmetic_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;

        return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
}

// 6.5.7/6.5.10-13
// Each of the operands shall have integer type.
static tree_type* cprog_check_bitwise_op(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_integral_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;
        if (!cprog_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;

        return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
}

// 6.5.13/6.5.14 logical and/or operator
// Each of the operands shall have scalar type
static tree_type* cprog_check_log_op(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_scalar_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;
        if (!cprog_require_scalar_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;

        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

static void cprog_invalid_binop_operands(
        const cprog* self, tree_binop_kind opcode, tree_location loc)
{
        cerror(self->error_manager, CES_ERROR, loc,
                "invalid operands to binary '%s'", cget_binop_string(opcode));
}

// 6.5.8 Relational operators
// One of the following shall hold:
// - both operands have real type;
// - both operands are pointers to qualified or unqualified versions of
// compatible object types; or
// - both operands are pointers to qualified or unqualified versions of
// compatible incomplete types
static tree_type* cprog_check_relational_op(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_real(lt) && tree_type_is_real(rt))
        {
                if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                        cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        }
        else if (tree_type_is_object_pointer(lt) && tree_type_is_object_pointer(rt))
        {
                if (!cprog_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
        }
        else
        {
                cprog_invalid_binop_operands(self, opcode, loc);
                return NULL;
        }

        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

// 6.5.9 Equality operators
// One of the following shall hold:
// - both operands have arithmetic type;
// - both operands are pointers to qualified or unqualified versions of compatible types;
// - one operand is a pointer to an object or incomplete type and the other is a pointer to a
// qualified or unqualified version of void; or
// - one operand is a pointer and the other is a null pointer constant.
static tree_type* cprog_check_compare_op(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
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
                        cerror(self->error_manager, CES_ERROR, loc,
                                "comparison of distinct pointer types");
                        return NULL;
                }
                else if (!cprog_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
        }
        else
        {
                cprog_invalid_binop_operands(self, opcode, loc);
                return NULL;
        }
        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

// 6.5.16.2 Compound assignment
// For the operators += and -= only, either the left operand shall be a pointer to an object
// type and the right shall have integer type, or the left operand shall have qualified or
// unqualified arithmetic type and the right shall have arithmetic type.
static tree_type* cprog_check_add_sub_assign_op(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_array_function_to_pointer_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (!cprog_require_modifiable_lvalue(self, *lhs))
                return NULL;

        if (tree_type_is_object_pointer(lt))
        {
                if (!cprog_require_integral_expr_type(self, rt, rl))
                        return NULL;
        }
        else if (tree_type_is_arithmetic(lt))
        {
                if (!cprog_require_arithmetic_expr_type(self, rt, rl))
                        return NULL;
        }
        else
        {
                cprog_invalid_binop_operands(self, opcode, loc);
                return NULL;
        }

        return lt;
}

static tree_type* cprog_check_mul_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_mul_div_op(self, TBK_MUL, loc, lhs, rhs);
}

static tree_type* cprog_check_div_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_mul_div_op(self, TBK_DIV, loc, lhs, rhs);
}

// 6.5.5 multiplicative
// The operands of the % operator shall have integer type
static tree_type* cprog_check_mod_op_ex(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                return NULL;
        if (!cprog_require_integral_expr_type(self, rt, tree_get_expr_loc(*rhs)))
                return NULL;

        return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
}

static tree_type* cprog_check_mod_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_mod_op_ex(self, TBK_MOD, loc, lhs, rhs);
}

// 6.5.6 additive
// For addition, either both operands shall have arithmetic type, or one operand shall be a
// pointer to an object type and the other shall have integer type.
// (Incrementing is equivalent to adding 1.)
static tree_type* cprog_check_add_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        else if (tree_type_is_object_pointer(lt))
        {
                if (!cprog_require_integral_expr_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        else if (tree_type_is_object_pointer(rt))
        {
                if (!cprog_require_integral_expr_type(self, lt, tree_get_expr_loc(*lhs)))
                        return NULL;

                return rt;
        }
        cprog_invalid_binop_operands(self, TBK_ADD, loc);
        return NULL;
}

// 6.5.6 additive
// For subtraction, one of the following shall hold :
// - both operands have arithmetic type;
// - both operands are pointers to qualified or unqualified versions
// of compatible object types; or
// - the left operand is a pointer to an object type and the right operand has integer type.
// (Decrementing is equivalent to subtracting 1.)
static tree_type* cprog_check_sub_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        else if (tree_type_is_object_pointer(lt))
        {
                if (tree_type_is_object_pointer(rt) && tree_types_are_same(lt, rt))
                        return lt;

                if (!cprog_require_integral_expr_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        cprog_invalid_binop_operands(self, TBK_SUB, loc);
        return NULL;
}

static tree_type* cprog_check_shl_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_bitwise_op(self, TBK_SHL, loc, lhs, rhs);
}

static tree_type* cprog_check_shr_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_bitwise_op(self, TBK_SHR, loc, lhs, rhs);
}

static tree_type* cprog_check_le_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_relational_op(self, TBK_LE, loc, lhs, rhs);
}

static tree_type* cprog_check_gr_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_relational_op(self, TBK_GR, loc, lhs, rhs);
}

static tree_type* cprog_check_leq_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_relational_op(self, TBK_LEQ, loc, lhs, rhs);
}

static tree_type* cprog_check_geq_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_relational_op(self, TBK_GEQ, loc, lhs, rhs);
}

static tree_type* cprog_check_eq_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_compare_op(self, TBK_EQ, loc, lhs, rhs);
}

static tree_type* cprog_check_neq_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_compare_op(self, TBK_NEQ, loc, lhs, rhs);
}

static tree_type* cprog_check_and_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_bitwise_op(self, TBK_AND, loc, lhs, rhs);
}

static tree_type* cprog_check_xor_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_bitwise_op(self, TBK_XOR, loc, lhs, rhs);
}

static tree_type* cprog_check_or_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_bitwise_op(self, TBK_OR, loc, lhs, rhs);
}

static tree_type* cprog_check_log_and_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_log_op(self, TBK_LOG_AND, loc, lhs, rhs);
}

static tree_type* cprog_check_log_or_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_log_op(self, TBK_LOG_OR, loc, lhs, rhs);
}

// returns false if pointee of lt has less qualifiers than pointee of rt
static bool cprog_check_pointer_qualifier_discartion(
        cprog* self, tree_type* lt, tree_type* rt, tree_location loc)
{
        lt = tree_get_pointer_target(lt);
        rt = tree_get_pointer_target(rt);

        tree_type_quals rq = tree_get_type_quals(rt);
        if (rq == TTQ_UNQUALIFIED)
                return true;

        tree_type_quals diff = TTQ_UNQUALIFIED;
        tree_type_quals lq = tree_get_type_quals(lt);

        if ((rq & TTQ_CONST) && !(lq & TTQ_CONST))
                diff |= TTQ_CONST;
        if ((rq & TTQ_VOLATILE) && !(lq & TTQ_VOLATILE))
                diff |= TTQ_VOLATILE;
        if ((rq & TTQ_RESTRICT) && !(lq & TTQ_RESTRICT))
                diff |= TTQ_RESTRICT;
        
        if (diff == TTQ_UNQUALIFIED)
                return true;

        char quals[64];
        cqet_qual_string(diff, quals);
        cerror(self->error_manager, CES_ERROR, loc,
                "assignment discards '%s' qualifier", quals);
        return false;
}

static bool cprog_check_assignment_pointer_types(
        cprog* self, tree_type* lt, tree_type* rt, tree_location loc)
{
        S_ASSERT(tree_type_is_object_pointer(lt) && tree_type_is_object_pointer(rt));
        tree_type* ltarget = tree_get_unqualified_type(tree_get_pointer_target(lt));
        tree_type* rtarget = tree_get_unqualified_type(tree_get_pointer_target(rt));

        if (tree_types_are_same(ltarget, rtarget)
            || (tree_type_is_incomplete(ltarget) && tree_type_is_void(rtarget))
            || (tree_type_is_incomplete(rtarget) && tree_type_is_void(ltarget)))
        {
                return cprog_check_pointer_qualifier_discartion(self, lt, rt, loc);
        }

        cerror(self->error_manager, CES_ERROR, loc,
               "assignment from incompatible pointer type");
        return false;
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
static tree_type* cprog_check_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        tree_type* lt = cprog_perform_array_function_to_pointer_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_expr_loc(*rhs);

        if (!cprog_require_modifiable_lvalue(self, *lhs))
                return NULL;

        if (tree_type_is_arithmetic(lt))
        {
                if (!cprog_require_arithmetic_expr_type(self, rt, rl))
                        return NULL;
        }
        else if (tree_type_is_record(lt))
        {
                if (!cprog_require_record_expr_type(self, rt, rl))
                        return NULL;
                if (!cprog_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
        }
        else if (tree_type_is_object_pointer(lt) && tree_expr_is_null_pointer_constant(*rhs))
                ; // nothing to check
        else if (tree_type_is_object_pointer(lt) && tree_type_is_object_pointer(rt))
        {
                if (!cprog_check_assignment_pointer_types(self, lt, rt, loc))
                        return NULL;
        }
        else
        {
                cprog_invalid_binop_operands(self, TBK_ASSIGN, loc);
                return NULL;
        }

        *rhs = cprog_build_impl_cast(self, *rhs, lt);
        return lt;
}

static tree_type* cprog_check_add_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_add_sub_assign_op(self, TBK_ADD_ASSIGN, loc, lhs, rhs);
}

static tree_type* cprog_check_sub_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_check_add_sub_assign_op(self, TBK_SUB_ASSIGN, loc, lhs, rhs);
}

static tree_type* cprog_check_mul_assign_op(
        cprog* self, tree_expr**lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_mul_div_op(self, TBK_MUL_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_div_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_mul_div_op(self, TBK_DIV_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_mod_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_mod_op_ex(self, TBK_MOD_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_shl_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_bitwise_op(self, TBK_SHL_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_shr_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_bitwise_op(self, TBK_SHR_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_and_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_bitwise_op(self, TBK_AND_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_xor_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_bitwise_op(self, TBK_XOR_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_or_assign_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        return cprog_require_modifiable_lvalue(self, *lhs)
                ? cprog_check_bitwise_op(self, TBK_OR_ASSIGN, loc, lhs, rhs)
                : NULL;
}

static tree_type* cprog_check_comma_op(
        cprog* self, tree_expr** lhs, tree_expr** rhs, tree_location loc)
{
        cprog_perform_unary_conversion(self, lhs); 
        return cprog_perform_unary_conversion(self, rhs);
}

S_STATIC_ASSERT(TBK_SIZE == 31, "Binop table needs an update");

static tree_type* (*ccheck_binop_table[TBK_SIZE])(
        cprog*, tree_expr**, tree_expr**, tree_location) =
{
        NULL, // TBK_UNKNOWN
        cprog_check_mul_op, // TBK_MUL
        cprog_check_div_op, // TBK_DIV
        cprog_check_mod_op, // TBK_MOD
        cprog_check_add_op, // TBK_ADD
        cprog_check_sub_op, // TBK_SUB
        cprog_check_shl_op, // TBK_SHL
        cprog_check_shr_op, // TBK_SHR
        cprog_check_le_op, // TBK_LE
        cprog_check_gr_op, // TBK_GR
        cprog_check_leq_op, // TBK_LEQ
        cprog_check_geq_op, // TBK_GEQ
        cprog_check_eq_op, // TBK_EQ
        cprog_check_neq_op, // TBK_NEQ
        cprog_check_and_op, // TBK_AND
        cprog_check_xor_op, // TBK_XOR
        cprog_check_or_op, // TBK_OR
        cprog_check_log_and_op, // TBK_LOG_AND
        cprog_check_log_or_op, // TBK_LOG_OR
        cprog_check_assign_op, // TBK_ASSIGN
        cprog_check_add_assign_op, // TBK_ADD_ASSIGN
        cprog_check_sub_assign_op, // TBK_SUB_ASSIGN
        cprog_check_mul_assign_op, // TBK_MUL_ASSIGN
        cprog_check_div_assign_op, // TBK_DIV_ASSIGN
        cprog_check_mod_assign_op, // TBK_MOD_ASSIGN
        cprog_check_shl_assign_op, // TBK_SHL_ASSIGN
        cprog_check_shr_assign_op, // TBK_SHR_ASSIGN
        cprog_check_and_assign_op, // TBK_AND_ASSIGN
        cprog_check_xor_assign_op, // TBK_XOR_ASSIGN
        cprog_check_or_assign_op, // TBK_OR_ASSIGN
        cprog_check_comma_op, // TBK_COMMA
};

// returns type of the binary operator
static inline tree_type* cprog_check_binop(
        cprog* self, tree_binop_kind opcode, tree_location loc, tree_expr** lhs, tree_expr** rhs)
{
        if (!lhs || !rhs)
                return NULL;

        S_ASSERT(opcode > 0 && opcode < TBK_SIZE && "invalid binop kind");
        return ccheck_binop_table[opcode](self, lhs, rhs, loc);
}

extern tree_expr* cprog_build_binop(
        cprog* self, tree_location loc, tree_binop_kind opcode, tree_expr* lhs, tree_expr* rhs)
{
        tree_type* t = cprog_check_binop(self, opcode, loc, &lhs, &rhs);
        if (!t)
                return NULL;

        return tree_new_binop(self->context, TVK_RVALUE, t, loc, opcode, lhs, rhs);
}

static tree_type* cprog_check_conditional_operator_pointer_types(
        cprog* self, tree_expr* lhs, tree_expr* rhs, tree_location loc)
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
                        cerror(self->error_manager, CES_ERROR, loc,
                               "pointer type mismatch in conditional expression");
                        return NULL;
                }
                quals = tree_get_type_quals(ltarget) | tree_get_type_quals(rtarget);
        }
        else
                return NULL;

        target = tree_new_qual_type(self->context, quals, target);
        return cprog_build_pointer(self, TTQ_UNQUALIFIED, target);
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
extern tree_expr* cprog_build_conditional(
        cprog* self,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs)
{
        if (!condition || !lhs || !rhs)
                return NULL;

        tree_type* ct = tree_get_expr_type(condition);
        if (!cprog_require_scalar_expr_type(self, ct, tree_get_expr_loc(condition)))
                return NULL;

        tree_type* t = NULL;
        tree_type* lt = tree_get_expr_type(lhs);
        tree_type* rt = tree_get_expr_type(rhs);
        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                t = cprog_perform_usual_arithmetic_conversion(self, &lhs, &rhs);
        else if (tree_type_is_record(lt) && tree_type_is_record(rt))
        {
                if (!cprog_require_compatible_expr_types(self, lt, rt, loc))
                        return NULL;
                t = lt;
        }
        else if (tree_type_is_void(lt) && tree_type_is_void(rt))
                t = lt;
        else if ((t = cprog_check_conditional_operator_pointer_types(self, lhs, rhs, loc)))
                ;
        else
        {
                cerror(self->error_manager, CES_ERROR, loc,
                       "type mismatch in conditional expression");
                return NULL;
        }

        S_ASSERT(t);
        return tree_new_conditional_expr(self->context, TVK_RVALUE, t, loc, condition, lhs, rhs);
}

extern tree_designation* cprog_build_designation(cprog* self)
{
        return tree_new_designation(self->context, NULL);
}

extern tree_designation* cprog_finish_designation(
        cprog* self,
        tree_expr* initializer_list,
        tree_designation* designation,
        tree_expr* designation_initializer)
{
        tree_set_designation_initializer(designation, designation_initializer);
        tree_add_init_designation(initializer_list, designation);
        return designation;
}

extern bool cprog_add_empty_designation(cprog* self, tree_expr* initializer_list)
{
        return cprog_finish_designation(self, initializer_list, cprog_build_designation(self), NULL);
}

extern tree_type* cprog_get_designation_type(cprog* self, tree_designation* d)
{
        return cprog_get_designator_type(self, tree_get_designation_last(d));
}

extern tree_designator* cprog_build_member_designator(
        cprog* self, tree_location loc, tree_type* t, tree_id name)
{
        t = tree_desugar_type(t);
        if (!cprog_require_record_expr_type(self, t, loc))
                return NULL;

        tree_decl* record = tree_get_decl_type_entity(t);
        tree_decl* m = cprog_require_member_decl(self, loc, record, name);
        if (!m)
                return NULL;
        
        return tree_new_member_designator(self->context, m);
}

extern tree_designator* cprog_build_array_designator(
        cprog* self, tree_location loc, tree_type* t, tree_expr* index)
{
        t = tree_desugar_type(t);
        if (!cprog_require_array_expr_type(self, t, loc))
                return NULL;

        tree_type* eltype = tree_get_array_eltype(t);
        if (!eltype)
                return NULL;

        return tree_new_array_designator(self->context, eltype, index);
}

extern tree_designator* cprog_finish_designator(
        cprog* self, tree_designation* designation, tree_designator* designator)
{
        tree_add_designation_designator(designation, designator);
        return designator;
}

extern tree_type* cprog_get_designator_type(cprog* self, tree_designator* d)
{
        if (!d)
                return NULL;

        if (tree_designator_is(d, TDK_DES_ARRAY))
                return tree_get_array_designator_type(d);

        tree_decl* member = tree_get_member_designator_decl(d);
        return tree_get_decl_type(member);
}

extern tree_expr* cprog_build_init_expr(cprog* self, tree_location loc)
{
        return tree_new_init_expr(self->context, loc);
}