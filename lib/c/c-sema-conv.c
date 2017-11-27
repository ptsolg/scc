#include "scc/c/c-sema-conv.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-info.h"

extern tree_expr* csema_new_impl_cast(csema* self, tree_expr* e, tree_type* t)
{
        tree_type* et = tree_desugar_type(tree_get_expr_type(e));

        if (et == t || tree_types_are_same(tree_get_unqualified_type(et),
                                           tree_get_unqualified_type(t)))
        {
                return e;
        }
  
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

extern tree_type* csema_default_argument_promotion(csema* self, tree_expr** e)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(*e));
        if (!tree_type_is(t, TTK_BUILTIN))
                return t;

        if (tree_type_is_integer(t))
                return csema_integer_promotion(self, e);

        if (tree_builtin_type_is(t, TBTK_FLOAT))
        {
                tree_type* double_type = csema_new_builtin_type(self,
                        TTQ_UNQUALIFIED, TBTK_DOUBLE);

                *e = csema_new_impl_cast(self, *e, double_type);
                return double_type;
        }

        return t;
}

static bool csema_check_pointer_qualifier_discartion(
        csema* self, tree_type* lt, tree_type* rt, cassign_conv_result* r)
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

        r->discarded_quals = diff;
        r->kind = CACRK_QUAL_DISCARTION;
        return false;
}

static bool csema_pointer_operands_are_compatible(csema* self, tree_type* lt, tree_type* rt)
{
        if (tree_types_are_same(lt, rt))
                return true;

        if ((tree_type_is_incomplete(lt) || tree_type_is_object(lt)) && tree_type_is_void(rt))
                return true;

        return (tree_type_is_incomplete(rt) || tree_type_is_object(rt)) && tree_type_is_void(lt);
}

static bool csema_check_assignment_pointer_types(
        csema* self, tree_type* lt, tree_type* rt, cassign_conv_result* r)
{
        S_ASSERT(tree_type_is_pointer(lt) && tree_type_is_pointer(rt));

        tree_type* ltarget = tree_get_unqualified_type(tree_get_pointer_target(lt));
        tree_type* rtarget = tree_get_unqualified_type(tree_get_pointer_target(rt));

        if (csema_pointer_operands_are_compatible(self, ltarget, rtarget))
                return csema_check_pointer_qualifier_discartion(self, lt, rt, r);

        r->kind = CACRK_INCOMPATIBLE_POINTERS;
        return false;
}

static inline tree_type* cassign_conv_error(
        cassign_conv_result_kind k, cassign_conv_result* r)
{
        r->kind = k;
        return NULL;
}

extern tree_type* csema_assignment_conversion(
        csema* self, tree_type* lt, tree_expr** rhs, cassign_conv_result* r)
{
        S_ASSERT(r);

        tree_type* rt = csema_unary_conversion(self, rhs);
        if (tree_type_is_arithmetic(lt))
        {
                if (!tree_type_is_arithmetic(rt))
                        return cassign_conv_error(CACRK_RHS_NOT_AN_ARITHMETIC, r);
        }
        else if (tree_type_is_record(lt))
        {
                if (!tree_type_is_record(rt))
                        return cassign_conv_error(CACRK_RHS_NOT_A_RECORD, r);
                if (!tree_types_are_same(lt, rt))
                        return cassign_conv_error(CACRK_INCOMPATIBLE_RECORDS, r);
        }
        else if (tree_type_is_object_pointer(lt) && tree_expr_is_null_pointer_constant(*rhs))
                ; // nothing to check
        else if (tree_type_is_object_pointer(lt) && tree_type_is_object_pointer(rt))
        {
                if (!csema_check_assignment_pointer_types(self, lt, rt, r))
                        return NULL;
        }
        else 
                return cassign_conv_error(CACRK_INCOMPATIBLE, r);

        *rhs = csema_new_impl_cast(self, *rhs, lt);
        r->kind = CACRK_COMPATIBLE;
        return lt;
}