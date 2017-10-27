#include "c-prog-exp.h"
#include "c-prog-type.h"
#include "c-prog-conversions.h"

extern bool cprog_require_object_pointer_exp_type(
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
extern bool cprog_require_function_pointer_exp_type(
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

extern bool cprog_require_integral_exp_type(
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

extern bool cprog_require_real_exp_type(
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

extern bool cprog_require_record_exp_type(
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

extern bool cprog_require_array_exp_type(
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

extern bool cprog_require_scalar_exp_type(
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

extern bool cprog_require_arithmetic_exp_type(
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

extern bool cprog_require_real_or_object_pointer_exp_type(
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
        const cprog* self, const tree_exp* e)
{
        tree_type* t = tree_desugar_type(tree_get_exp_type(e));
        if (!tree_exp_is_lvalue(e) && tree_get_type_kind(t) != TTK_FUNCTION)
        {
                cerror(self->error_manager, CES_ERROR, tree_get_exp_loc(e),
                        "expression must be an lvalue or function designator");
                return false;
        }
        return true;
}

extern bool cprog_require_modifiable_lvalue(const cprog* self, const tree_exp* e)
{
        if (!tree_exp_is_modifiable_lvalue(e))
        {
                cerror(self->error_manager, CES_ERROR, tree_get_exp_loc(e),
                        "expression must be a modifiable lvalue");
                return false;
        }
        return true;
}

extern bool cprog_require_compatible_exp_types(
        const cprog* self, const tree_type* a, const tree_type* b, tree_location l)
{
        if (!tree_types_are_compatible(a, b))
        {
                // todo: warning?
                cerror(self->error_manager, CES_ERROR, l, "types are not compatible");
                return false;
        }
        return true;
}

extern tree_exp* cprog_build_paren_exp(cprog* self, tree_location loc, tree_exp* exp)
{
        if (!exp)
                return NULL;

        return tree_new_paren_exp(
                self->context,
                tree_get_exp_value_kind(exp),
                tree_get_exp_type(exp),
                loc,
                exp);
}

extern tree_exp* cprog_build_decl_exp(cprog* self, tree_location loc, tree_id name)
{
        tree_decl* d = cprog_require_local_decl(self, loc, TDK_UNKNOWN, name);
        if (!d)
                return NULL; // unknown decl

        tree_decl_kind  dk = tree_get_decl_kind(d);
        tree_value_kind vk = TVK_LVALUE;
        tree_type*      t  = tree_desugar_type(tree_get_decl_type(d));

        if (dk == TDK_ENUMERATOR || dk == TDK_FUNCTION)
                vk = TVK_RVALUE;
        else if (dk != TDK_VAR)
                return NULL; // unknown decl kind

        return tree_new_decl_exp(self->context, vk, t, loc, d);
}

extern tree_exp* cprog_build_floating_literal(cprog* self, tree_location loc, float v)
{
        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_FLOAT);
        return tree_new_floating_literal(self->context, t, loc, v);
}

extern tree_exp* cprog_build_floating_lliteral(cprog* self, tree_location loc, ldouble v)
{
        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_DOUBLE);
        return tree_new_floating_lliteral(self->context, t, loc, v);
}

extern tree_exp* cprog_build_character_literal(cprog* self, tree_location loc, int c)
{
        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT8);
        return tree_new_character_literal(self->context, t, loc, c);
}

extern tree_exp* cprog_build_integer_literal(
        cprog* self, tree_location loc, suint64 v, bool signed_, bool ext)
{
        tree_builtin_type_kind btk = ext ? TBTK_INT64 : TBTK_INT32;
        if (!signed_)
                btk = ext ? TBTK_UINT64 : TBTK_UINT32;

        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, btk);
        return tree_new_integer_literal(self->context, t, loc, v);
}

extern tree_exp* cprog_build_string_literal(cprog* self, tree_location loc, tree_id ref)
{
        tree_type* t = cprog_build_pointer(self, TTQ_UNQUALIFIED,
                cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT8));
        return tree_new_string_literal(self->context, t, loc, ref);
}

extern tree_exp* cprog_build_subscript_exp(
        cprog* self, tree_location loc, tree_exp* lhs, tree_exp* rhs)
{
        if (!lhs || !rhs)
                return NULL;

        tree_type*    lt = tree_desugar_type(cprog_perform_unary_conversion(self, &lhs));
        tree_type*    rt = tree_desugar_type(tree_get_exp_type(rhs));
        tree_location rl = tree_get_exp_loc(rhs);
        tree_type*    t  = NULL;

        // c99 6.5.2.1 
        // 1. One of the expressions shall have type "pointer to object type",
        //    the other expression shall have integer type, and the result has type "type".
        if (tree_type_is_pointer(lt))
        {
                if (!cprog_require_integral_exp_type(self, rt, rl))
                        return NULL;

                t = tree_get_pointer_target(lt);
        }
        else if (cprog_require_object_pointer_exp_type(self, rt, rl))
        {
                if (!cprog_require_integral_exp_type(self, lt, tree_get_exp_loc(lhs)))
                        return NULL;

                t = tree_get_pointer_target(rt);
        }
        else
                return NULL;

        return tree_new_subscript_exp(self->context, TVK_LVALUE, t, loc, lhs, rhs);
}

extern tree_exp* cprog_build_call_exp(
        cprog* self, tree_location loc, tree_exp* lhs, objgroup* args)
{
        if (!lhs || !args)
                return NULL;

        // c99 6.5.2.2
        // 1. The expression that denotes the called function shall have type pointer to function
        //    returning void or returning an object type other than an array type.
        // 2. If the expression that denotes the called function has a type that includes a prototype,
        //    the number of arguments shall agree with the number of parameters. Each argument shall
        //    have a type such that its value may be assigned to an object with the unqualified version
        //    of the type of its corresponding parameter.
        tree_type* t = cprog_perform_unary_conversion(self, &lhs);
        if (!cprog_require_function_pointer_exp_type(self, t, tree_get_exp_loc(lhs)))
                return NULL;

        t = tree_get_pointer_target(tree_desugar_ctype(t));
        if (tree_get_function_type_nparams(t) != objgroup_size(args))
                return NULL;
  
        tree_exp* call = tree_new_call_exp(self->context, TVK_RVALUE,
                tree_get_function_restype(t), loc, lhs);

        tree_type** typeit = tree_get_function_type_begin(t);
        OBJGROUP_FOREACH(args, tree_exp**, it)
        {
                tree_exp* arg = cprog_build_impl_cast(self, *it, *typeit);
                tree_add_call_arg(call, arg);
                typeit++;
        }
        return call;
}

extern tree_exp* cprog_build_member_exp(
        cprog*        self,
        tree_location loc,
        tree_exp*     lhs,
        tree_id       id,
        tree_location id_loc,
        bool          is_arrow)
{
        if (!lhs)
                return NULL;

        tree_location lhs_loc = tree_get_exp_loc(lhs);
        tree_type* t = cprog_perform_array_function_to_pointer_conversion(self, &lhs);

        if (is_arrow)
        {
                t = cprog_perform_lvalue_conversion(self, &lhs);
                if (!cprog_require_object_pointer_exp_type(self, t, lhs_loc))
                        return NULL;

                t = tree_get_pointer_target(t);
        }
        t = tree_desugar_type(t);
        if (!cprog_require_record_exp_type(self, t, lhs_loc))
                return NULL;
        
        tree_decl* record = tree_get_decl_type_entity(t);
        tree_decl* m      = cprog_require_member_decl(self, id_loc, record, id);
        if (!m)
                return NULL;

        // c99 6.5.2.3
        // 1. The first operand of the . operator shall have a qualified or unqualified
        //    structure or union type, and the second operand shall name a member of that type.
        // 2. The first operand of the -> operator shall have type "pointer to qualified 
        //    or unqualified structure" or "pointer to qualified or unqualified union",
        //    and the second operand shall name a member of the type pointed to.

        return tree_new_member_exp(self->context, 
                TVK_LVALUE, tree_get_decl_type(m), loc, lhs, m, is_arrow);
}

static tree_type* cprog_check_inc_dec_op(cprog* self, tree_exp** exp)
{
        tree_type* t = cprog_perform_array_function_to_pointer_conversion(self, exp);
        if (!cprog_require_real_or_object_pointer_exp_type(self, t, tree_get_exp_loc(*exp)))
                return NULL;
        if (!cprog_require_modifiable_lvalue(self, *exp))
                return NULL;
        return t;
}

static tree_type* cprog_check_address_op(cprog* self, tree_exp** exp)
{
        if (!cprog_require_lvalue_or_function_designator(self, *exp))
                return NULL;

        return cprog_build_pointer(self, TTQ_UNQUALIFIED, tree_get_exp_type(*exp));
}

static tree_type* cprog_check_dereference_op(cprog* self, tree_exp** exp, tree_value_kind* vk)
{
        tree_type* t      = tree_desugar_type(cprog_perform_unary_conversion(self, exp));
        tree_type_kind tk = tree_get_type_kind(t);

        if (!cprog_require_object_pointer_exp_type(self, t, tree_get_exp_loc(*exp)))
                return NULL;
 
        t = tree_get_pointer_target(t);
        *vk = TVK_LVALUE;
        return t;
}

static tree_type* cprog_check_log_not_op(cprog* self, tree_exp** exp)
{
        tree_type* t = cprog_perform_unary_conversion(self, exp);
        if (!cprog_require_scalar_exp_type(self, t, tree_get_exp_loc(*exp)))
                return NULL;

        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

static tree_type* cprog_check_plus_minus_op(cprog* self, tree_exp** exp)
{
        tree_type* t = cprog_perform_unary_conversion(self, exp);
        if (!cprog_require_arithmetic_exp_type(self, t, tree_get_exp_loc(*exp)))
                return NULL;

        return cprog_perform_integer_promotion(self, exp);
}

static tree_type* cprog_check_not_op(cprog* self, tree_exp** exp)
{
        tree_type* t = cprog_perform_unary_conversion(self, exp);
        if (!cprog_require_integral_exp_type(self, t, tree_get_exp_loc(*exp)))
                return NULL;

        return cprog_perform_integer_promotion(self, exp);
}

extern tree_exp* cprog_build_unop(
        cprog* self, tree_location loc, tree_unop_kind opcode, tree_exp* exp)
{
        if (!exp)
                return NULL;

        // c99 6.5.2.4/6.5.3.1
        // 1. The operand of the postfix (or prefix) increment or decrement operator shall have qualified or
        //    unqualified real or pointer type and shall be a modifiable lvalue.
        //
        // 6.5.3.2 address inderection
        // 1. The operand of the unary & operator shall be either a function designator, the result of a
        //    [] or unary * operator, or an lvalue that designates an object that is not a bit - field and is
        //    not declared with the register storage - class specifier.
        // 2. The operand of the unary * operator shall have pointer type.
        //
        // 6.5.3.3 unary arithmetic
        // 1. The operand of the unary + or - operator shall have arithmetic type;
        //    of the ~ operator, integer type; of the ! operator, scalar type.

        tree_type* t       = NULL;
        tree_value_kind vk = TVK_RVALUE;

        switch (opcode)
        {
                case TUK_POST_INC:
                case TUK_POST_DEC:
                case TUK_PRE_INC:
                case TUK_PRE_DEC:
                        t = cprog_check_inc_dec_op(self, &exp);
                        break;

                case TUK_ADDRESS:
                        t = cprog_check_address_op(self, &exp);
                        break;

                case TUK_DEREFERENCE:
                        t = cprog_check_dereference_op(self, &exp, &vk);
                        break;

                case TUK_LOG_NOT:
                        t = cprog_check_log_not_op(self, &exp);
                        break;

                case TUK_PLUS:
                case TUK_MINUS:
                        t = cprog_check_plus_minus_op(self, &exp);
                        break;

                case TUK_NOT:
                        t = cprog_check_not_op(self, &exp);
                        break;

                default:
                        // unknown unop
                        S_UNREACHABLE();
        }
        if (!t)
                return NULL;

        return tree_new_unop(self->context, vk, t, loc, opcode, exp);
}

extern tree_exp* cprog_build_sizeof(cprog* self, tree_location loc, csizeof_rhs* rhs)
{
        if (!rhs)
                return NULL;
        //sizeof(cprog_build_sizeof);
        // c99 6.5.3.4
        // 1. The sizeof operator shall not be applied to an expression that has function type or an
        //    incomplete type, to the parenthesized name of such a type, or to an expression that
        //    designates a bit - field member.
        tree_type* rt = rhs->unary ? tree_get_exp_type(rhs->exp) : rhs->type;
        if (tree_type_is(rt, TTK_FUNCTION))
        {
                cerror(self->error_manager, CES_ERROR, rhs->loc,
                        "operand of sizeof may not be a function");
                return NULL;
        }
        if (!cprog_require_complete_type(self, rhs->loc, rt))
                return NULL;

        // todo: bitfields
        tree_builtin_type_kind btk = TBTK_UINT32;
        if (tree_platform_is(tree_get_module_platform(self->module), TPK_X64))
                btk = TBTK_UINT64;

        tree_type* t = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, btk);
        return tree_new_sizeof_exp(self->context, t, loc, rhs->pointer, rhs->unary);
}

extern tree_exp* cprog_build_cast(
        cprog* self, tree_location loc, tree_type* type, tree_exp* exp)
{
        if (!type || !exp)
                return NULL;
        // c99 6.5.4
        // 2. Unless the type name specifies a void type, the type name shall specify qualified or
        //    unqualified scalar type and the operand shall have scalar type.
        // 3. Conversions that involve pointers, other than where permitted by the constraints of
        //    6.5.16.1, shall be specified by means of an explicit cast.
        tree_type* et = tree_desugar_type(cprog_perform_unary_conversion(self, &exp));
        if (!tree_type_is_void(type))
        {
                if (!cprog_require_scalar_exp_type(self, type, loc))
                        return NULL;
                if (!cprog_require_scalar_exp_type(self, et, tree_get_exp_loc(exp)))
                        return NULL;
        }

        tree_value_kind vk = tree_get_exp_value_kind(exp);
        return tree_new_explicit_cast_exp(self->context, vk, loc, type, exp);
}

static tree_type* cprog_check_mul_div_op(cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_arithmetic_exp_type(self, lt, tree_get_exp_loc(*lhs)))
                return NULL;
        if (!cprog_require_arithmetic_exp_type(self, rt, tree_get_exp_loc(*rhs)))
                return NULL;

        return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
}

static tree_type* cprog_check_mod_op(cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_integral_exp_type(self, lt, tree_get_exp_loc(*lhs)))
                return NULL;
        if (!cprog_require_integral_exp_type(self, rt, tree_get_exp_loc(*rhs)))
                return NULL;
 
        return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
}

static tree_type* cprog_check_add_op(
        cprog* self, tree_exp** lhs, tree_exp** rhs, tree_location loc)
{
        tree_type*    lt = cprog_perform_unary_conversion(self, lhs);
        tree_type*    rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_exp_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        else if (tree_type_is_object_pointer(lt))
        {
                if (!cprog_require_integral_exp_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        else if (tree_type_is_object_pointer(rt))
        {
                if (!cprog_require_integral_exp_type(self, lt, tree_get_exp_loc(*lhs)))
                        return NULL;

                return rt;
        }
        cerror(self->error_manager, CES_ERROR, loc, "invalid operands to binary '+'");
        return NULL;
}

static tree_type* cprog_check_sub_op(
        cprog* self, tree_exp** lhs, tree_exp** rhs, tree_location loc)
{
        tree_type*    lt = cprog_perform_unary_conversion(self, lhs);
        tree_type*    rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_exp_loc(*rhs);

        if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        else if (tree_type_is_object_pointer(lt))
        {
                if (tree_type_is_object_pointer(rt) && tree_types_are_compatible(lt, rt))
                        return lt;

                if (!cprog_require_integral_exp_type(self, rt, rl))
                        return NULL;

                return lt;
        }
        cerror(self->error_manager, CES_ERROR, loc, "invalid operands to binary '-'");
        return NULL;
}

static tree_type* cprog_check_bitwise_op(cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_integral_exp_type(self, rt, tree_get_exp_loc(*rhs)))
                return NULL;
        if (!cprog_require_integral_exp_type(self, lt, tree_get_exp_loc(*lhs)))
                return NULL;

        return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
}

static tree_type* cprog_check_log_op(cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type* lt = cprog_perform_unary_conversion(self, lhs);
        tree_type* rt = cprog_perform_unary_conversion(self, rhs);

        if (!cprog_require_scalar_exp_type(self, rt, tree_get_exp_loc(*rhs)))
                return NULL;
        if (!cprog_require_scalar_exp_type(self, lt, tree_get_exp_loc(*lhs)))
                return NULL;

        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

static tree_type* cprog_check_relational_op(
        cprog* self, tree_exp** lhs, tree_exp** rhs, tree_location loc)
{
        tree_type*    lt = cprog_perform_unary_conversion(self, lhs);
        tree_type*    rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_exp_loc(*rhs);

        if (tree_type_is_real(lt))
        {
                if (!cprog_require_real_exp_type(self, rt, rl))
                        return NULL;

                if (tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt))
                        cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        }
        else if (tree_type_is_pointer(lt))
        {
                if (!cprog_require_object_pointer_exp_type(self, rt, rl))
                        return NULL;
                if (!cprog_require_compatible_exp_types(self, lt, rt, loc))
                        return NULL;
        }
        else
                return NULL;

        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_INT32);
}

static tree_type* cprog_check_compare_op(
        cprog* self, tree_exp** lhs, tree_exp** rhs, tree_location loc)
{
        tree_type*    lt = cprog_perform_unary_conversion(self, lhs);
        tree_type*    rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_exp_loc(*rhs);

        if (tree_type_is_arithmetic(lt))
        {
                if (!cprog_require_arithmetic_exp_type(self, rt, rl))
                        return NULL;

                return cprog_perform_usual_arithmetic_conversion(self, lhs, rhs);
        }
        else if (tree_type_is_pointer(lt) && tree_type_is_pointer(rt))
        {
                if (tree_types_are_compatible(lt, rt))
                        return lt;

                if (tree_type_is_void_pointer(lt) && tree_type_is_void_pointer(rt))
                        return lt;

                /*
                todo:
                if (nullptr(lhs) && nullptr(rhs)
                        return lt;
                */
        }
        cerror(self->error_manager, CES_ERROR, loc, "invalid operands to binary '=='");
        return NULL;
}

static tree_type* cprog_check_simple_assignment_op(
        cprog* self, tree_exp** lhs, tree_exp** rhs, tree_location loc)
{
        tree_type*    lt = cprog_perform_array_function_to_pointer_conversion(self, lhs);
        tree_type*    rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_exp_loc(*rhs);

        if (!cprog_require_modifiable_lvalue(self, *lhs))
                return NULL;

        if (tree_type_is_arithmetic(lt))
        {
                if (!cprog_require_arithmetic_exp_type(self, rt, rl))
                        return NULL;

                *rhs = cprog_build_impl_cast(self, *rhs, lt);
                return lt;
        }
        else if (tree_type_is_record(lt))
        {
                if (!cprog_require_record_exp_type(self, rt, rl))
                        return NULL;
                if (!cprog_require_compatible_exp_types(self, lt, rt, loc))
                        return NULL;

                return lt;
        }
        else if (tree_type_is_pointer(lt) && tree_type_is_pointer(rt))
        {
                bool same_quals = tree_get_type_quals(lt) == tree_get_type_quals(rt);
                if (tree_types_are_compatible(lt, rt) && same_quals)
                        return lt;
                if ((tree_type_is_void_pointer(lt) || tree_type_is_void_pointer(rt)) && same_quals)
                        return lt;
                /*
                todo:
                if (nullptr(lhs) && nullptr(rhs)
                        return lt;
                */
        }
        cerror(self->error_manager, CES_ERROR, loc, "invalid operands to binary '='");
        return NULL;
}

static tree_type* cprog_check_add_sub_assignment_op(
        cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type*    lt = cprog_perform_array_function_to_pointer_conversion(self, lhs);
        tree_type*    rt = cprog_perform_unary_conversion(self, rhs);
        tree_location rl = tree_get_exp_loc(*rhs);

        if (!cprog_require_modifiable_lvalue(self, *lhs))
                return NULL;

        if (tree_type_is_object_pointer(lt))
        {
                if (!cprog_require_integral_exp_type(self, rt, rl))
                        return NULL;
        }
        else if (tree_type_is_arithmetic(lt))
        {
                if (!cprog_require_arithmetic_exp_type(self, rt, rl))
                        return NULL;
        }
        else
                return NULL;

        return lt;
}

static tree_type* cprog_check_comma_op(cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        cprog_perform_unary_conversion(self, lhs);
        cprog_perform_unary_conversion(self, rhs);
        return cprog_build_builtin_type(self, TTQ_UNQUALIFIED, TBTK_VOID);
}

extern tree_exp* cprog_build_binop(
        cprog* self, tree_location loc, tree_binop_kind opcode, tree_exp* lhs, tree_exp* rhs)
{
        if (!lhs || !rhs)
                return NULL;

        // 6.5.5 multiplicative
        // Each of the operands shall have arithmetic type.
        // The operands of the % operator shall have integer type

        // 6.5.6 additive
        // For addition, either both operands shall have arithmetic type, or one operand shall be a
        // pointer to an object type and the other shall have integer type.
        // (Incrementing is equivalent to adding 1.)
        // For subtraction, one of the following shall hold :
        // � both operands have arithmetic type;
        // � both operands are pointers to qualified or unqualified versions of compatible object
        // types; or
        // � the left operand is a pointer to an object type and the right operand has integer type.
        // (Decrementing is equivalent to subtracting 1.)

        // 6.5.7 shift
        // Each of the operands shall have integer type.

        // 6.5.8 Relational operators
        // One of the following shall hold:
        // � both operands have real type;
        // � both operands are pointers to qualified or unqualified versions of compatible object
        // types; or
        // � both operands are pointers to qualified or unqualified versions of compatible
        // incomplete types

        // 6.5.9 Equality operators
        // One of the following shall hold:
        // � both operands have arithmetic type;
        // � both operands are pointers to qualified or unqualified versions of compatible types;
        // � one operand is a pointer to an object or incomplete type and the other is a pointer to a
        // qualified or unqualified version of void; or
        // � one operand is a pointer and the other is a null pointer constant.

        // 6.5.10 Bitwise AND operator
        // Each of the operands shall have integer type.

        // 6.5.11 Bitwise exclusive OR operator
        // Each of the operands shall have integer type.

        // 6.5.12 Bitwise inclusive OR operator
        // Each of the operands shall have integer type.

        // 6.5.13 Logical AND operator
        // Each of the operands shall have scalar type.

        // Logical OR operator
        // Each of the operands shall have scalar type.

        // 6.5.16 Assignment operators
        // An assignment operator shall have a modifiable lvalue as its left operand.

        // 6.5.16.1 Simple assignment
        // One of the following shall hold:
        // � the left operand has qualified or unqualified arithmetic type and the right has
        // arithmetic type;
        // � the left operand has a qualified or unqualified version of a structure or union type
        // compatible with the type of the right;
        // � both operands are pointers to qualified or unqualified versions of compatible types,
        // and the type pointed to by the left has all the qualifiers of the type pointed to by the
        // right;
        // � one operand is a pointer to an object or incomplete type and the other is a pointer to a
        // qualified or unqualified version of void, and the type pointed to by the left has all
        // the qualifiers of the type pointed to by the right;
        // � the left operand is a pointer and the right is a null pointer constant; or
        // � the left operand has type _Bool and the right is a pointer.

        // 6.5.16.2 Compound assignment
        // For the operators += and -= only, either the left operand shall be a pointer to an object
        // type and the right shall have integer type, or the left operand shall have qualified or
        // unqualified arithmetic type and the right shall have arithmetic type.
        //
        // For the other operators, each operand shall have arithmetic type consistent with those
        // allowed by the corresponding binary operator.
        tree_type* t = NULL;
        switch (opcode)
        {
                case TBK_MUL_ASSIGN:
                case TBK_DIV_ASSIGN:
                        if (!cprog_require_modifiable_lvalue(self, lhs))
                                return NULL;
                case TBK_MUL:
                case TBK_DIV:
                        t = cprog_check_mul_div_op(self, &lhs, &rhs);
                        break;

                case TBK_MOD_ASSIGN:
                        if (!cprog_require_modifiable_lvalue(self, lhs))
                                return NULL;
                case TBK_MOD:
                        t = cprog_check_mod_op(self, &lhs, &rhs);
                        break;

                case TBK_ADD:
                        t = cprog_check_add_op(self, &lhs, &rhs, loc);
                        break;

                case TBK_SUB:
                        t = cprog_check_sub_op(self, &lhs, &rhs, loc);
                        break;

                case TBK_SHL_ASSIGN:
                case TBK_SHR_ASSIGN:
                case TBK_AND_ASSIGN:
                case TBK_XOR_ASSIGN:
                case TBK_OR_ASSIGN:
                        if (!cprog_require_modifiable_lvalue(self, lhs))
                                return NULL;
                case TBK_SHL:
                case TBK_SHR:
                case TBK_AND:
                case TBK_OR:
                case TBK_XOR:
                        t = cprog_check_bitwise_op(self, &lhs, &rhs);
                        break;

                case TBK_LOG_AND:
                case TBK_LOG_OR:
                        t = cprog_check_log_op(self, &lhs, &rhs);
                        break;

                case TBK_LE:
                case TBK_GR:
                case TBK_LEQ:
                case TBK_GEQ:
                        t = cprog_check_relational_op(self, &lhs, &rhs, loc);
                        break;

                case TBK_EQ:
                case TBK_NEQ:
                        t = cprog_check_compare_op(self, &lhs, &rhs, loc);
                        break;

                case TBK_ASSIGN:
                        t = cprog_check_simple_assignment_op(self, &lhs, &rhs, loc);
                        break;

                case TBK_ADD_ASSIGN:
                case TBK_SUB_ASSIGN:
                        t = cprog_check_add_sub_assignment_op(self, &lhs, &rhs);
                        break;

                case TBK_COMMA:
                        t = cprog_check_comma_op(self, &lhs, &rhs);
                        break;

                default:
                        S_UNREACHABLE();
        }
        if (!t)
                return NULL;

        return tree_new_binop(self->context, TVK_RVALUE, t, loc, opcode, lhs, rhs);
}

extern tree_exp* cprog_build_conditional(
        cprog*        self,
        tree_location loc,
        tree_exp*     condition,
        tree_exp*     lhs,
        tree_exp*     rhs)
{
        if (!condition || !lhs || !rhs)
                return NULL;

        // 6.5.15 Conditional operator
        // The first operand shall have scalar type.
        // One of the following shall hold for the second and third operands:
        // � both operands have arithmetic type;
        // � both operands have the same structure or union type;
        // � both operands have void type;
        // � both operands are pointers to qualified or unqualified versions of compatible types;
        // � one operand is a pointer and the other is a null pointer constant; or
        // � one operand is a pointer to an object or incomplete type and the other is a pointer to a
        //   qualified or unqualified version of void.
        tree_type* ct = tree_get_exp_type(condition);
        if (!cprog_require_scalar_exp_type(self, ct, tree_get_exp_loc(condition)))
                return NULL;


        tree_type* t = tree_get_exp_type(lhs);
        return tree_new_conditional_exp(self->context, TVK_RVALUE, t, loc, condition, lhs, rhs);
}

extern tree_const_exp* cprog_build_const_exp(cprog* self, tree_exp* root)
{
        if (!root)
                return NULL;

        // 6.6 Constant expressions
        // Constant expressions shall not contain assignment, increment, decrement, function-call,
        // or comma operators, except when they are contained within a subexpression that is not
        // evaluated.
        // 
        // Each constant expression shall evaluate to a constant that is in the range of representable
        // values for its type.

        return tree_new_const_exp(self->context, root);
}

extern tree_designation* cprog_build_designation(cprog* self)
{
        return tree_new_designation(self->context, NULL);
}

extern tree_designation* cprog_finish_designation(
        cprog*            self,
        tree_exp*         initializer_list,
        tree_designation* designation,
        tree_exp*         designation_initializer)
{
        tree_set_designation_initializer(designation, designation_initializer);
        tree_add_init_designation(initializer_list, designation);
        return designation;
}

extern bool cprog_add_empty_designation(cprog* self, tree_exp* initializer_list)
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
        if (!cprog_require_record_exp_type(self, t, loc))
                return NULL;

        tree_decl* record = tree_get_decl_type_entity(t);
        tree_decl* m      = cprog_require_member_decl(self, loc, record, name);
        if (!m)
                return NULL;
        
        return tree_new_member_designator(self->context, m);
}

extern tree_designator* cprog_build_array_designator(
        cprog* self, tree_location loc, tree_type* t, tree_const_exp* index)
{
        t = tree_desugar_type(t);
        if (!cprog_require_array_exp_type(self, t, loc))
                return NULL;

        tree_type* eltype = tree_get_array_eltype(t);
        if (!eltype)
                return NULL;

        return tree_new_array_designator(self->context, eltype, index);
}

extern tree_designator* cprog_finish_designator(
        cprog* self, tree_designation* designation, tree_designator*  designator)
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

extern tree_exp* cprog_build_init_exp(cprog* self, tree_location loc)
{
        return tree_new_init_exp(self->context, loc);
}