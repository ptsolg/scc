#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-info.h"

extern tree_expr* csema_new_impl_cast(csema* self, tree_expr* e, tree_type* t)
{
        tree_type* et = tree_desugar_type(tree_get_expr_type(e));

        if (et == t)
                return e;
        if (tree_type_is(et, TTK_BUILTIN) && tree_type_is(t, TTK_BUILTIN))
                if (tree_get_builtin_type_kind(et) == tree_get_builtin_type_kind(t))
                        return e;
  
        return tree_new_implicit_cast_expr(self->context,
                tree_get_expr_value_kind(e), tree_get_expr_loc(e), t, e);
}

extern tree_type* csema_lvalue_conversion(csema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (tree_expr_is_lvalue(*e) && tree_get_type_kind(t) != TTK_ARRAY)
        {
                t = tree_new_qual_type(self->context, TTQ_UNQUALIFIED, t);
                *e = csema_new_impl_cast(self, *e, t);
                tree_set_expr_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* csema_array_to_pointer_conversion(csema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (tree_expr_is_lvalue(*e) && tree_type_is(t, TTK_ARRAY))
        {
                tree_type* eltype = tree_get_array_eltype(t);
                t = csema_new_pointer(self, TTQ_UNQUALIFIED, eltype);
                *e = csema_new_impl_cast(self, *e, t);
                tree_set_expr_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* csema_function_to_pointer_conversion(csema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (tree_type_is(t, TTK_FUNCTION))
        {
                t = csema_new_pointer(self, TTQ_UNQUALIFIED, t);
                *e = csema_new_impl_cast(self, *e, t);
                tree_set_expr_value_kind(*e, TVK_RVALUE);
        }
        return t;
}

extern tree_type* csema_integer_promotion(csema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        tree_type_kind tk = tree_get_type_kind(t);
        if (tk != TTK_BUILTIN)
                return t;

        tree_builtin_type_kind btk = tree_get_builtin_type_kind(t);
        tree_type_quals quals = tree_get_type_quals(t);

        if (btk == TBTK_INT8 || btk == TBTK_INT16)
        {
                t = csema_new_builtin_type(self, quals, TBTK_INT32);
                *e = csema_new_impl_cast(self, *e, t);
        }
        else if (btk == TBTK_UINT8 || btk == TBTK_UINT16)
        {
                t = csema_new_builtin_type(self, quals, TBTK_UINT32);
                *e = csema_new_impl_cast(self, *e, t);
        }
        return t;
}

extern tree_type* csema_array_function_to_pointer_conversion(csema* self, tree_expr** e)
{
        csema_function_to_pointer_conversion(self, e);
        return csema_array_to_pointer_conversion(self, e);
}

extern tree_type* csema_unary_conversion(csema* self, tree_expr** e)
{
        csema_array_function_to_pointer_conversion(self, e);
        return csema_lvalue_conversion(self, e);
}

extern tree_type* csema_usual_arithmetic_conversion(
        csema* self, tree_expr** lhs, tree_expr** rhs)
{
        tree_type* lt = csema_integer_promotion(self, lhs);
        tree_type* rt = csema_integer_promotion(self, rhs);

        S_ASSERT(tree_type_is_arithmetic(lt));
        S_ASSERT(tree_type_is_arithmetic(rt)); 
        if (tree_types_are_same(lt, rt))
                return lt;

        tree_builtin_type_kind lk = tree_get_builtin_type_kind(lt);
        tree_builtin_type_kind rk = tree_get_builtin_type_kind(rt);

        if (cget_type_rank(lt) > cget_type_rank(rt))
        {
                tree_builtin_type_kind tk = rk;
                rk = lk;
                lk = tk;
                tree_type* tt = rt;
                rt = lt;
                lt = tt;
                tree_expr** te = rhs;
                rhs = lhs;
                lhs = te;
        }

        if (tree_type_is_floating(rt))
        {
                *lhs = csema_new_impl_cast(self, *lhs, rt);
                return rt;
        }

        uint lsize = tree_get_builtin_type_size(self->target, lk);
        uint rsize = tree_get_builtin_type_size(self->target, rk);
        if (rsize > lsize)
        {
                *lhs = csema_new_impl_cast(self, *lhs, rt);
                return rt;
        }

        S_ASSERT(lsize == rsize);
        if (tree_type_is_unsigned_integer(lt) == tree_type_is_unsigned_integer(rt))
                return rt;

        tree_type* counterpart = rt;
        if (tree_type_is_signed_integer(rt))
                counterpart = csema_new_builtin_type(self,
                        TTQ_UNQUALIFIED, tree_get_integer_counterpart(rt));

        *lhs = csema_new_impl_cast(self, *lhs, counterpart);
        *rhs = csema_new_impl_cast(self, *rhs, counterpart);

        return counterpart;
}