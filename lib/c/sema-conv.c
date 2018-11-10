#include "scc/c/sema-conv.h"
#include "scc/c/sema-type.h"
#include "misc.h"
#include "scc/tree/target.h"

extern tree_expr* c_sema_new_impl_cast(c_sema* self, tree_expr* e, tree_type* t)
{
        tree_type* et = tree_desugar_type(tree_get_expr_type(e));

        if (c_sema_types_are_same(self, tree_get_modified_type(et), tree_get_modified_type(t)))
                return e;
  
        return tree_new_cast_expr(self->context,
                tree_get_expr_value_kind(e), tree_get_expr_loc(e), t, e, true);
}

extern tree_type* c_sema_lvalue_conversion(c_sema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (tree_expr_is_lvalue(*e) && tree_get_type_kind(t) != TTK_ARRAY)
        {
                // the desugared expressions can be still lvalues,
                // but we don't need them in value-checking
                tree_set_expr_value_kind(tree_desugar_expr(*e), TVK_RVALUE);
                t = tree_new_qualified_type(self->context, t, TTQ_UNQUALIFIED);
                *e = c_sema_new_impl_cast(self, *e, t);
                tree_set_expr_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* c_sema_array_to_pointer_conversion(c_sema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (tree_expr_is_lvalue(*e) && tree_type_is(t, TTK_ARRAY))
        {
                tree_type* eltype = tree_get_array_eltype(t);
                t = c_sema_new_pointer_type(self, TTQ_UNQUALIFIED, eltype);
                *e = c_sema_new_impl_cast(self, *e, t);
                tree_set_expr_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* c_sema_function_to_pointer_conversion(c_sema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (tree_type_is(t, TTK_FUNCTION))
        {
                t = c_sema_new_pointer_type(self, TTQ_UNQUALIFIED, t);
                *e = c_sema_new_impl_cast(self, *e, t);
                tree_set_expr_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

static tree_type* c_sema_get_type_for_integer_promotion(c_sema* self, tree_type* type)
{
        tree_type* t = tree_desugar_type(type);
        tree_type_kind tk = tree_get_type_kind(t);
        if (tk != TTK_BUILTIN)
                return t;

        tree_builtin_type_kind btk = tree_get_builtin_type_kind(t);
        tree_type_quals quals = tree_get_type_quals(t);

        if (btk == TBTK_INT8 || btk == TBTK_INT16)
                return c_sema_get_builtin_type(self, quals, TBTK_INT32);
        else if (btk == TBTK_UINT8 || btk == TBTK_UINT16)
                return c_sema_get_builtin_type(self, quals, TBTK_UINT32);
        return t;
}

extern tree_type* c_sema_integer_promotion(c_sema* self, tree_expr** e)
{
        tree_type* expr_type = tree_desugar_type(tree_get_expr_type(*e));
        tree_type* promoted_type = c_sema_get_type_for_integer_promotion(self, expr_type);
        if (expr_type == promoted_type)
                return expr_type;

        *e = c_sema_new_impl_cast(self, *e, promoted_type);
        return promoted_type;
}

extern tree_type* c_sema_array_function_to_pointer_conversion(c_sema* self, tree_expr** e)
{
        c_sema_function_to_pointer_conversion(self, e);
        return c_sema_array_to_pointer_conversion(self, e);
}

extern tree_type* c_sema_unary_conversion(c_sema* self, tree_expr** e)
{
        c_sema_array_function_to_pointer_conversion(self, e);
        return c_sema_lvalue_conversion(self, e);
}

static tree_type* c_sema_get_type_for_usual_arithmetic_conversion(
        c_sema* self, tree_type* lhs, tree_type* rhs)
{
        // make rhs greater type
        if (c_get_type_rank(lhs) > c_get_type_rank(rhs))
        {
                tree_type* tmp = rhs;
                rhs = lhs;
                lhs = tmp;
        }

        if (tree_type_is_floating(rhs))
                return rhs;

        tree_builtin_type_kind lk = tree_declared_type_is(lhs, TDK_ENUM)
                ? TBTK_INT32 : tree_get_builtin_type_kind(lhs);
        tree_builtin_type_kind rk = tree_declared_type_is(rhs, TDK_ENUM)
                ? TBTK_INT32 : tree_get_builtin_type_kind(rhs);

        uint lsize = tree_get_builtin_type_size(self->target, lk);
        uint rsize = tree_get_builtin_type_size(self->target, rk);
        if (rsize > lsize)
                return rhs;

        assert(lsize == rsize);
        if (tree_type_is_unsigned_integer(lhs) == tree_type_is_unsigned_integer(rhs))
                return rhs;

        if (tree_type_is_signed_integer(rhs))
                return c_sema_get_builtin_type(self,
                        TTQ_UNQUALIFIED, tree_get_integer_counterpart(rhs));

        return rhs;
}

extern tree_type* c_sema_usual_arithmetic_conversion(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, bool convert_lhs)
{
        tree_type* lt = convert_lhs 
                ? c_sema_integer_promotion(self, lhs)
                : c_sema_get_type_for_integer_promotion(self, tree_get_expr_type(*lhs));
        tree_type* rt = c_sema_integer_promotion(self, rhs);

        assert(tree_type_is_arithmetic(lt));
        assert(tree_type_is_arithmetic(rt)); 
        if (c_sema_types_are_same(self, lt, rt))
                return lt;

        tree_type* result = c_sema_get_type_for_usual_arithmetic_conversion(self, lt, rt);
        if (convert_lhs)
                *lhs = c_sema_new_impl_cast(self, *lhs, result);
        *rhs = c_sema_new_impl_cast(self, *rhs, result);
        return result;
}

extern tree_type* c_sema_default_argument_promotion(c_sema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (!tree_type_is(t, TTK_BUILTIN))
                return t;

        if (tree_type_is_integer(t))
                return c_sema_integer_promotion(self, e);

        if (tree_builtin_type_is(t, TBTK_FLOAT))
        {
                tree_type* double_type = c_sema_get_builtin_type(self,
                        TTQ_UNQUALIFIED, TBTK_DOUBLE);

                *e = c_sema_new_impl_cast(self, *e, double_type);
                return double_type;
        }

        return t;
}

static bool c_sema_check_attribute_discartion(
        c_sema* self, tree_type* lt, tree_type* rt, c_assignment_conversion_result* r)
{
        if (tree_type_is(lt, TTK_FUNCTION) && tree_type_is(rt, TTK_FUNCTION)
                && tree_func_type_is_transaction_safe(lt)
                && !tree_func_type_is_transaction_safe(rt))
        {
                r->kind = CACRK_RHS_TRANSACTION_UNSAFE;
                return false;
        }

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

        r->discarded_quals = diff;
        r->kind = CACRK_QUAL_DISCARTION;
        return false;
}

static bool c_sema_check_pointer_assignment(
        c_sema* self, tree_type* lt, tree_type* rt, c_assignment_conversion_result* r)
{
        tree_type* ltarget = tree_ignore_paren_types(tree_get_pointer_target(lt));
        tree_type* rtarget = tree_ignore_paren_types(tree_get_pointer_target(rt));

        if (c_sema_types_are_compatible(self, ltarget, rtarget, true)
                || (tree_type_is_incomplete_or_object(ltarget) && tree_type_is_void(rtarget))
                || (tree_type_is_incomplete_or_object(rtarget) && tree_type_is_void(ltarget)))
        {
                return c_sema_check_attribute_discartion(self, ltarget, rtarget, r);
        }

        r->kind = CACRK_INCOMPATIBLE_POINTERS;
        return false;
}

static inline tree_type* c_assignment_conversion_error(
        c_assignment_conversion_result* r, c_assignment_conversion_result_kind k)
{
        r->kind = k;
        return NULL;
}

extern tree_type* c_sema_assignment_conversion(
        c_sema* self, tree_type* lt, tree_expr** rhs, c_assignment_conversion_result* r)
{
        assert(r);

        lt = tree_desugar_type(lt);
        tree_type* rt = c_sema_unary_conversion(self, rhs);

        if (tree_type_is_arithmetic(lt))
        {
                if (!tree_type_is_arithmetic(rt))
                        return c_assignment_conversion_error(r, CACRK_RHS_NOT_AN_ARITHMETIC);
        }
        else if (tree_type_is_record(lt))
        {
                if (!tree_type_is_record(rt))
                        return c_assignment_conversion_error(r, CACRK_RHS_NOT_A_RECORD);
                if (!c_sema_types_are_same(self, lt, rt))
                        return c_assignment_conversion_error(r, CACRK_INCOMPATIBLE_RECORDS);
        }
        else if (tree_type_is_pointer(lt) && tree_type_is_pointer(rt))
        {
                if (!c_sema_check_pointer_assignment(self, lt, rt, r))
                        return NULL;
        }
        else if (tree_type_is_pointer(lt) && tree_expr_is_null_pointer_constant(self->context, *rhs))
                ;
        else if (/*lt is bool*/0 && tree_type_is_pointer(rt))
                UNREACHABLE(); // todo _Bool
        else
                return c_assignment_conversion_error(r, CACRK_INCOMPATIBLE);

        *rhs = c_sema_new_impl_cast(self, *rhs, lt);
        r->kind = CACRK_COMPATIBLE;
        return lt;
}