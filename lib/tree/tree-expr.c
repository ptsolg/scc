#include "scc/tree/tree-expr.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-eval.h"
#include "scc/tree/tree-target.h"

extern tree_expr* tree_new_expr(
        tree_context* context,
        tree_expr_kind kind,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        ssize size)
{
        S_ASSERT(size >= sizeof(struct _tree_expr_base));
        tree_expr* e = tree_allocate_node(context, size);
        if (!e)
                return NULL;

        tree_set_expr_kind(e, kind);
        tree_set_expr_value_kind(e, value_kind);
        tree_set_expr_type(e, type);
        tree_set_expr_loc(e, loc);
        return e;
}

extern tree_expr* tree_new_binop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_binop_kind kind,
        tree_expr* lhs,
        tree_expr* rhs)
{
        tree_expr* e = tree_new_expr(context, TEK_BINARY, value_kind, type, loc,
                sizeof(struct _tree_binop));
        if (!e)
                return NULL;

        tree_set_binop_kind(e, kind);
        tree_set_binop_lhs(e, lhs);
        tree_set_binop_rhs(e, rhs);
        return e;
}

extern tree_expr* tree_new_unop(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_unop_kind kind,
        tree_expr* operand)
{
        tree_expr* e = tree_new_expr(context, TEK_UNARY, value_kind, type, loc,
                sizeof(struct _tree_unop));
        if (!e)
                return NULL;

        tree_set_unop_kind(e, kind);
        tree_set_unop_operand(e, operand);
        return e;
}

extern tree_expr* tree_new_cast_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_expr* expr,
        bool is_implicit)
{
        tree_expr* e = tree_new_expr(context, TEK_CAST, value_kind, type, loc,
                sizeof(struct _tree_cast_expr));
        if (!e)
                return NULL;

        tree_set_cast_operand(e, expr);
        tree_set_cast_implicit(e, is_implicit);
        return e;
}

extern tree_expr* tree_new_call_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs)
{
        tree_expr* e = tree_new_expr(context, TEK_CALL, value_kind, type, loc,
                sizeof(struct _tree_call_expr));
        if (!e)
                return NULL;

        tree_set_call_lhs(e, lhs);
        tree_init_array(&e->call.args);
        return e;
}

extern serrcode tree_add_call_arg(tree_expr* self, tree_context* context, tree_expr* arg)
{
        S_ASSERT(arg);
        return tree_array_append_ptr(context, &self->call.args, arg);
}

extern tree_expr* tree_new_subscript_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs,
        tree_expr* rhs)
{
        tree_expr* e = tree_new_expr(context, TEK_SUBSCRIPT, value_kind, type, loc,
                sizeof(struct _tree_subscript_expr));
        if (!e)
                return NULL;

        tree_set_subscript_lhs(e, lhs);
        tree_set_subscript_rhs(e, rhs);
        return e;
}

extern tree_expr* tree_new_conditional_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs)
{
        tree_expr* e = tree_new_expr(context, TEK_CONDITIONAL, value_kind, type, loc,
                sizeof(struct _tree_conditional_expr));
        if (!e)
                return NULL;

        tree_set_conditional_condition(e, condition);
        tree_set_conditional_lhs(e, lhs);
        tree_set_conditional_rhs(e, rhs);
        return e;
}

extern tree_expr* tree_new_integer_literal(
        tree_context* context, tree_type* type, tree_location loc, suint64 value)
{
        tree_expr* e = tree_new_expr(context, TEK_INTEGER_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_integer_literal_expr));
        if (!e)
                return NULL;

        tree_set_integer_literal(e, value);
        return e;
}

extern tree_expr* tree_new_character_literal(
        tree_context* context, tree_type* type, tree_location loc, int value)
{
        tree_expr* e = tree_new_expr(context, TEK_CHARACTER_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_character_literal_expr));
        if (!e)
                return NULL;

        tree_set_character_literal(e, value);
        return e;
}

extern tree_expr* tree_new_floating_literal(
        tree_context* context,
        tree_type* type,
        tree_location loc,
        const float_value* value)
{
        tree_expr* e = tree_new_expr(context, TEK_FLOATING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_floating_literal_expr));
        if (!e)
                return NULL;

        tree_set_floating_literal_value(e, value);
        return e;
}

extern tree_expr* tree_new_string_literal(
        tree_context* context,
        tree_value_kind value,
        tree_type* type,
        tree_location loc,
        tree_id id)
{
        tree_expr* e = tree_new_expr(context, TEK_STRING_LITERAL, value, type, loc,
                sizeof(struct _tree_string_literal_expr));
        if (!e)
                return NULL;

        tree_set_string_literal(e, id);
        return e;
}

extern tree_expr* tree_new_decl_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_decl* decl)
{
        tree_expr* e = tree_new_expr(context, TEK_DECL, value_kind, type, loc, 
                sizeof(struct _tree_decl_expr));
        if (!e)
                return NULL;

        tree_set_decl_expr_entity(e, decl);
        return e;
}

extern tree_expr* tree_new_member_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* lhs,
        tree_decl* decl,
        bool is_arrow)
{
        tree_expr* e = tree_new_expr(context, TEK_MEMBER, value_kind, type, loc,
                sizeof(struct _tree_member_expr));
        if (!e)
                return NULL;

        tree_set_member_expr_decl(e, decl);
        tree_set_member_expr_arrow(e, is_arrow);
        tree_set_member_expr_lhs(e, lhs);
        return e;
}

extern tree_expr* tree_new_sizeof_expr(
        tree_context* context,
        tree_type* type,
        tree_location loc,
        void* operand,
        bool contains_type)
{
        tree_expr* e = tree_new_expr(context, TEK_SIZEOF, TVK_RVALUE, type, loc,
                sizeof(struct _tree_sizeof_expr));
        if (!e)
                return NULL;

        e->sizeof_expr.operand = operand;
        tree_set_sizeof_contains_type(e, contains_type);
        return e;
}

extern tree_expr* tree_new_paren_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        tree_expr* expr)
{
        tree_expr* e = tree_new_expr(context, TEK_PAREN, value_kind, type, loc,
                sizeof(struct _tree_paren_expr));
        if (!e)
                return NULL;
      
        tree_set_paren_expr(e, expr);
        return e;
}

extern tree_designator* tree_new_field_designator(
        tree_context* context, tree_location loc, tree_id field)
{
        tree_designator* d = tree_allocate_node(context, sizeof(tree_designator));
        if (!d)
                return NULL;

        d->is_field = true;
        d->field = field;
        tree_set_designator_loc(d, loc);
        return d;
}

extern tree_designator* tree_new_array_designator(
        tree_context* context, tree_location loc, tree_expr* index)
{
        tree_designator* d = tree_allocate_node(context, sizeof(tree_designator));
        if (!d)
                return NULL;

        d->is_field = false;
        d->index = index;
        tree_set_designator_loc(d, loc);
        return d;
}

extern tree_expr* tree_new_designation(tree_context* context, tree_expr* init)
{
        tree_expr* e = tree_new_expr(context, TEK_DESIGNATION,
                TVK_LVALUE, NULL, TREE_INVALID_LOC, sizeof(struct _tree_designation));
        if (!e)
                return NULL;

        tree_set_designation_init(e, init);
        tree_init_array(&e->designation.designators);
        return e;
}

extern serrcode tree_add_designation_designator(
        tree_expr* self, tree_context* context, tree_designator* d)
{
        S_ASSERT(d);
        return tree_array_append_ptr(context, &self->designation.designators, d);
}

extern tree_expr* tree_new_init_list_expr(tree_context* context, tree_location loc)
{
        tree_expr* e = tree_new_expr(context, 
                TEK_INIT_LIST, TVK_RVALUE, NULL, loc, sizeof(struct _tree_init_list_expr));
        if (!e)
                return NULL;

        tree_init_array(&e->init_list.exprs);
        tree_set_init_list_has_trailing_comma(e, false);
        return e;
}

extern serrcode tree_add_init_list_expr(tree_expr* self, tree_context* context, tree_expr* expr)
{
        S_ASSERT(expr);
        return tree_array_append_ptr(context, &self->init_list.exprs, expr);
}

extern tree_expr* tree_new_impl_init_expr(tree_context* context, tree_expr* expr)
{
        tree_expr* e = tree_new_expr(context, TEK_IMPL_INIT, TVK_RVALUE, NULL, TREE_INVALID_LOC,
                sizeof(struct _tree_impl_init_expr));
        if (!e)
                return NULL;

        tree_set_impl_init_expr(e, expr);
        return e;
}

extern bool tree_expr_is_literal(const tree_expr* self)
{
        switch (tree_get_expr_kind(self))
        {
                case TEK_CHARACTER_LITERAL:
                case TEK_FLOATING_LITERAL:
                case TEK_INTEGER_LITERAL:
                case TEK_STRING_LITERAL:
                        return true;

                default:
                        return false;
        }
}

extern const tree_expr* tree_ignore_ccasts(const tree_expr* self)
{
        while (tree_expr_is(self, TEK_CAST))
                self = tree_get_cast_operand(self);
        return self;
}

extern tree_expr* tree_ignore_impl_casts(tree_expr* self)
{
        while(tree_expr_is(self, TEK_CAST) && tree_cast_is_implicit(self))
                self = tree_get_cast_operand(self);
        return self;
}

extern const tree_expr* tree_ignore_impl_ccasts(const tree_expr* self)
{
        while (tree_expr_is(self, TEK_CAST) && tree_cast_is_implicit(self))
                self = tree_get_cast_operand(self);
        return self;
}

extern tree_expr* tree_ignore_paren_exprs(tree_expr* self)
{
        while (tree_expr_is(self, TEK_PAREN))
                self = tree_get_paren_expr(self);
        return self;
}

extern const tree_expr* tree_ignore_paren_cexps(const tree_expr* self)
{
        while (tree_expr_is(self, TEK_PAREN))
                self = tree_get_paren_expr(self);
        return self;
}

extern tree_expr* tree_desugar_expr(tree_expr* self)
{
        tree_expr* ignored = self;
        while ((ignored = tree_ignore_paren_exprs(tree_ignore_impl_casts(ignored))) != self)
                self = ignored;
        return ignored;
}

extern const tree_expr* tree_desugar_cexpr(const tree_expr* self)
{
        const tree_expr* ignored = self;
        while ((ignored = tree_ignore_paren_cexps(tree_ignore_impl_ccasts(ignored))) != self)
                self = ignored;
        return ignored;
}

extern bool tree_expr_is_null_pointer_constant(tree_context* context, const tree_expr* expr)
{
        while (tree_type_is_void_pointer(tree_get_expr_type(expr)))
        {
                if (tree_expr_is(expr, TEK_CAST))
                        expr = tree_get_cast_operand(expr);
                else
                        return false;
        }

        if (!tree_type_is_integer(tree_get_expr_type(expr)))
                return false;

        tree_eval_result result;
        if (!tree_eval_expr_as_integer(context, expr, &result))
                return false;

        return avalue_is_zero(&result.value);
}

extern bool tree_expr_designates_bitfield(const tree_expr* self)
{
        return false; // todo
}

