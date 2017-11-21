#include "scc/tree/tree-expr.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-eval.h"

extern tree_expr* tree_new_expr(
        tree_context* context,
        tree_expr_kind kind,
        tree_value_kind value_kind,
        tree_type* type,
        tree_location loc,
        ssize size)
{
        S_ASSERT(size >= sizeof(struct _tree_expr_base));
        tree_expr* e = tree_allocate(context, size);
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
        tree_expr* expr)
{
        tree_expr* e = tree_new_expr(context, TEK_UNARY, value_kind, type, loc,
                sizeof(struct _tree_unop));
        if (!e)
                return NULL;

        tree_set_unop_kind(e, kind);
        tree_set_unop_expr(e, expr);
        return e;
}

extern tree_expr* tree_new_EXPLIcit_cast_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_expr* expr)
{
        tree_expr* e = tree_new_expr(context, TEK_EXPLICIT_CAST, value_kind, type, loc,
                sizeof(struct _tree_cast_expr));
        if (!e)
                return NULL;

        tree_set_cast_expr(e, expr);
        return e;
}

extern tree_expr* tree_new_implicit_cast_expr(
        tree_context* context,
        tree_value_kind value_kind,
        tree_location loc,
        tree_type* type,
        tree_expr* expr)
{
        tree_expr* e = tree_new_expr(context, TEK_IMPLICIT_CAST, value_kind, type, loc,
                sizeof(struct _tree_cast_expr));
        if (!e)
                return NULL;

        tree_set_cast_expr(e, expr);
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
        dseq_init_ex_ptr(&_tree_get_call(e)->_args, tree_get_allocator(context));
        return e;
}

extern void tree_set_call_args(tree_expr* self, dseq* args)
{
        dseq* this_args = &_tree_get_call(self)->_args;
        dseq_dispose(this_args);
        dseq_move(this_args, args);
}

extern void tree_add_call_arg(tree_expr* self, tree_expr* arg)
{
        dseq_append_ptr(&_tree_get_call(self)->_args, arg);
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
        tree_context* context, tree_type* type, tree_location loc, float value)
{
        tree_expr* e = tree_new_expr(context, TEK_FLOATING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_floating_literal_expr));
        if (!e)
                return NULL;

        tree_set_floating_literal(e, value);
        return e;
}

extern tree_expr* tree_new_floating_lliteral(
        tree_context* context, tree_type* type, tree_location loc, ldouble value)
{
        tree_expr* e = tree_new_expr(context, TEK_FLOATING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_floating_literal_expr));
        if (!e)
                return NULL;

        tree_set_floating_lliteral(e, value);
        return e;
}

extern tree_expr* tree_new_string_literal(
        tree_context* context, tree_type* type, tree_location loc, tree_id ref)
{
        tree_expr* e = tree_new_expr(context, TEK_STRING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_string_literal_expr));
        if (!e)
                return NULL;

        tree_set_string_literal(e, ref);
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
        tree_context* context, tree_type* type, tree_location loc, void* rhs, bool is_unary)
{
        tree_expr* e = tree_new_expr(context, TEK_SIZEOF, TVK_RVALUE, type, loc,
                sizeof(struct _tree_sizeof_expr));
        if (!e)
                return NULL;

        if (is_unary)
                tree_set_sizeof_expr(e, rhs);
        else
                tree_set_sizeof_type(e, rhs);

        tree_set_sizeof_unary(e, is_unary);
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

extern tree_expr* tree_new_init_expr(tree_context* context, tree_location loc)
{
        tree_expr* e = tree_new_expr(context, TEK_INIT, TVK_RVALUE, NULL, loc,
                sizeof(struct _tree_init_expr));
        if (!e)
                return NULL;

        list_init(&_tree_get_init_expr(e)->_designations);
        return e;
}

extern void tree_add_init_designation(tree_expr* self, tree_designation* d)
{
        list_push_back(&_tree_get_init_expr(self)->_designations, &d->_node);
}

extern tree_designator* tree_new_member_designator(tree_context* context, tree_decl* member)
{
        tree_designator* d = tree_allocate(context, sizeof(tree_designator));
        if (!d)
                return NULL;

        list_node_init(&_tree_get_designator(d)->_node);
        tree_set_designator_kind(d, TDK_DES_MEMBER);
        tree_set_member_designator_decl(d, member);
        return d;
}

extern tree_designator* tree_new_array_designator(
        tree_context* context, tree_type* eltype, tree_expr* index)
{
        tree_designator* d = tree_allocate(context, sizeof(tree_designator));
        if (!d)
                return NULL;

        list_node_init(&_tree_get_designator(d)->_node);
        tree_set_designator_kind(d, TDK_DES_ARRAY);
        tree_set_array_designator_index(d, index);
        tree_set_array_designator_type(d, eltype);
        return d;
}

extern tree_designation* tree_new_designation(tree_context* context, tree_expr* initializer)
{
        tree_designation* d = tree_allocate(context, sizeof(tree_designation));
        if (!d)
                return NULL;

        list_node_init(&d->_node);
        list_init(&d->_designators);
        tree_set_designation_initializer(d, initializer);
        return d;
}

extern void tree_add_designation_designator(tree_designation* self, tree_designator* d)
{
        list_push_back(&self->_designators, &_tree_get_designator(d)->_node);
}

extern tree_expr* tree_new_impl_init_expr(tree_context* context, tree_expr* init)
{
        tree_expr* e = tree_new_expr(context, TEK_IMPL_INIT, TVK_RVALUE, NULL, TREE_INVALID_LOC,
                sizeof(struct _tree_impl_init_expr));
        if (!e)
                return NULL;

        tree_set_impl_init_expr(e, init);
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
        while (tree_expr_is(self, TEK_EXPLICIT_CAST)
            || tree_expr_is(self, TEK_IMPLICIT_CAST))
                self = tree_get_cast_expr(self);
        return self;
}

extern tree_expr* tree_ignore_impl_casts(tree_expr* self)
{
        while(tree_expr_is(self, TEK_IMPLICIT_CAST))
                self = tree_get_cast_expr(self);
        return self;
}

extern const tree_expr* tree_ignore_impl_ccasts(const tree_expr* self)
{
        while (tree_expr_is(self, TEK_IMPLICIT_CAST))
                self = tree_get_cast_expr(self);
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

extern const tree_expr* tree_desugar_cexp(const tree_expr* self)
{
        const tree_expr* ignored = self;
        while ((ignored = tree_ignore_paren_cexps(tree_ignore_impl_ccasts(ignored))) != self)
                self = ignored;
        return ignored;
}

extern bool tree_expr_is_null_pointer_constant(const tree_expr* self)
{
        while (tree_type_is_void_pointer(tree_get_expr_type(self)))
        {
                if (tree_expr_is(self, TEK_IMPLICIT_CAST)
                 || tree_expr_is(self, TEK_EXPLICIT_CAST))
                {
                        self = tree_get_cast_expr(self);
                }
                else
                        return false;
        }

        if (!tree_type_is_integer(tree_get_expr_type(self)))
                return false;

        tree_target_info t;
        tree_eval_info i;
        int_value v;

        tree_init_target_info(&t, TTARGET_X32);
        tree_init_eval_info(&i, &t);

        if (!tree_eval_as_integer(&i, self, &v))
                return false;

        return int_is_zero(&v);
}

extern bool tree_expr_designates_bitfield(const tree_expr* self)
{
        return false; // todo
}

