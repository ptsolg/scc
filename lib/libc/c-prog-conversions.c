#include "c-prog-conversions.h"
#include "c-prog-type.h"
#include "c-info.h"

extern tree_exp* cprog_build_impl_cast(cprog* self, tree_exp* e, tree_type* t)
{
        tree_type* et = tree_desugar_type(tree_get_exp_type(e));

        if (et == t)
                return e;
        if (tree_type_is(et, TTK_BUILTIN) && tree_type_is(t, TTK_BUILTIN))
                if (tree_get_builtin_type_kind(et) == tree_get_builtin_type_kind(t))
                        return e;
  
        return tree_new_implicit_cast_exp(self->context,
                tree_get_exp_value_kind(e), tree_get_exp_loc(e), t, e);
}

extern tree_type* cprog_perform_lvalue_conversion(cprog* self, tree_exp** e)
{
        tree_type* t = tree_desugar_type(tree_get_exp_type(*e));
        if (tree_exp_is_lvalue(*e) && tree_get_type_kind(t) != TTK_ARRAY)
        {
                t = tree_new_qual_type(self->context, TTQ_UNQUALIFIED, t);
                *e = cprog_build_impl_cast(self, *e, t);
                tree_set_exp_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* cprog_perform_array_to_pointer_conversion(cprog* self, tree_exp** e)
{
        tree_type* t = tree_desugar_type(tree_get_exp_type(*e));
        if (tree_exp_is_lvalue(*e) && tree_type_is(t, TTK_ARRAY))
        {
                tree_type* eltype = tree_get_array_eltype(t);
                t = cprog_build_pointer(self, TTQ_UNQUALIFIED, eltype);
                *e = cprog_build_impl_cast(self, *e, t);
                tree_set_exp_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* cprog_perform_function_to_pointer_conversion(cprog* self, tree_exp** e)
{
        tree_type* t = tree_desugar_type(tree_get_exp_type(*e));
        if (tree_type_is(t, TTK_FUNCTION))
        {
                t = cprog_build_pointer(self, TTQ_UNQUALIFIED, t);
                *e = cprog_build_impl_cast(self, *e, t);
                tree_set_exp_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* cprog_perform_integer_promotion(cprog* self, tree_exp** e)
{
        tree_type* t      = tree_desugar_type(tree_get_exp_type(*e));
        tree_type_kind tk = tree_get_type_kind(t);
        if (tk != TTK_BUILTIN)
                return t;

        tree_builtin_type_kind btk = tree_get_builtin_type_kind(t);
        tree_type_quals quals      = tree_get_type_quals(t);

        if (btk == TBTK_INT8 || btk == TBTK_INT16)
        {
                t = cprog_build_builtin_type(self, quals, TBTK_INT32);
                *e = cprog_build_impl_cast(self, *e, t);
        }
        else if (btk == TBTK_UINT8 || btk == TBTK_UINT16)
        {
                t = cprog_build_builtin_type(self, quals, TBTK_UINT32);
                *e = cprog_build_impl_cast(self, *e, t);
        }
        return t;
}

extern tree_type* cprog_perform_array_function_to_pointer_conversion(cprog* self, tree_exp** e)
{
        cprog_perform_function_to_pointer_conversion(self, e);
        return cprog_perform_array_to_pointer_conversion(self, e);
}

extern tree_type* cprog_perform_unary_conversion(cprog* self, tree_exp** e)
{
        cprog_perform_array_function_to_pointer_conversion(self, e);
        return cprog_perform_lvalue_conversion(self, e);
}

static tree_type* cprog_perform_float_conversion(cprog* self, tree_exp** flt, tree_exp** other)
{
        tree_type* f = tree_get_exp_type(*flt);
        *other = cprog_build_impl_cast(self, *other, f);
        return f;
}

static tree_type* cprog_perform_integer_conversion(cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type* lt = cprog_perform_integer_promotion(self, lhs);
        tree_type* rt = cprog_perform_integer_promotion(self, rhs);

        if (tree_builtin_types_are_same(lt, rt))
                return lt;

        bool lsigned = tree_type_is_signed_integer(lt);
        bool rsigned = tree_type_is_signed_integer(rt);
        int  lrank = cget_type_rank(lt);
        int  rrank = cget_type_rank(rt);
        tree_type* common = NULL;
        if (lsigned == rsigned)
        {
                if (lrank > rrank)
                {
                        *rhs = cprog_build_impl_cast(self, *rhs, lt);
                        common = lt;
                }
                else
                {
                        *lhs = cprog_build_impl_cast(self, *lhs, rt);
                        common = rt;
                }
        }
        else if (!lsigned && lrank >= rrank || lsigned && lrank > rrank)
        {
                *rhs = cprog_build_impl_cast(self, *rhs, lt);
                common = lt;
        }
        else if (!rsigned && rrank >= lrank || rsigned && rrank > lrank)
        {
                *lhs = cprog_build_impl_cast(self, *lhs, rt);
                common = rt;
        }
        else if (lsigned)
        {
                common = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, tree_get_integer_counterpart(lt));
                *rhs = cprog_build_impl_cast(self, *rhs, common);
        }
        else if (rsigned)
        {
                common = cprog_build_builtin_type(self, TTQ_UNQUALIFIED, tree_get_integer_counterpart(rt));
                *lhs = cprog_build_impl_cast(self, *lhs, common);
        }
        else
                S_UNREACHABLE();

        return common;

}

extern tree_type* cprog_perform_usual_arithmetic_conversion(
        cprog* self, tree_exp** lhs, tree_exp** rhs)
{
        tree_type* lt = tree_get_exp_type(*lhs);
        tree_type* rt = tree_get_exp_type(*rhs);
        if (tree_type_is_floating(lt))
                return cprog_perform_float_conversion(self, lhs, rhs);
        else if (tree_type_is_floating(rt))
                return cprog_perform_float_conversion(self, rhs, lhs);
        else
                return cprog_perform_integer_conversion(self, lhs, rhs);

        S_UNREACHABLE();
        return NULL;
}