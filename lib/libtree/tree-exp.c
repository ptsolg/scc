#include "tree-exp.h"
#include "tree-context.h"
#include "tree-type.h"

extern tree_exp* tree_new_exp(
        tree_context*   context,
        tree_exp_kind   kind,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        ssize           size)
{
        S_ASSERT(size >= sizeof(struct _tree_exp_base));
        tree_exp* e = tree_context_fast_allocate(context, size);
        if (!e)
                return NULL;

        tree_set_exp_kind(e, kind);
        tree_set_exp_value_kind(e, value_kind);
        tree_set_exp_type(e, type);
        tree_set_exp_loc(e, loc);
        return e;
}

extern tree_exp* tree_new_binop(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_binop_kind kind,
        tree_exp*       lhs,
        tree_exp*       rhs)
{
        tree_exp* e = tree_new_exp(context, TEK_BINARY, value_kind, type, loc,
                sizeof(struct _tree_binop));
        if (!e)
                return NULL;

        tree_set_binop_kind(e, kind);
        tree_set_binop_lhs(e, lhs);
        tree_set_binop_rhs(e, rhs);
        return e;
}

extern tree_exp* tree_new_unop(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_unop_kind  kind,
        tree_exp*       exp)
{
        tree_exp* e = tree_new_exp(context, TEK_UNARY, value_kind, type, loc,
                sizeof(struct _tree_unop));
        if (!e)
                return NULL;

        tree_set_unop_kind(e, kind);
        tree_set_unop_exp(e, exp);
        return e;
}

extern tree_exp* tree_new_explicit_cast_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_location   loc,
        tree_type*      type,
        tree_exp*       exp)
{
        tree_exp* e = tree_new_exp(context, TEK_EXPLICIT_CAST, value_kind, type, loc,
                sizeof(struct _tree_cast_exp));
        if (!e)
                return NULL;

        tree_set_cast_exp(e, exp);
        return e;
}

extern tree_exp* tree_new_implicit_cast_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_location   loc,
        tree_type*      type,
        tree_exp*       exp)
{
        tree_exp* e = tree_new_exp(context, TEK_IMPLICIT_CAST, value_kind, type, loc,
                sizeof(struct _tree_cast_exp));
        if (!e)
                return NULL;

        tree_set_cast_exp(e, exp);
        return e;
}

extern tree_exp* tree_new_call_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_exp*       lhs)
{
        tree_exp* e = tree_new_exp(context, TEK_CALL, value_kind, type, loc,
                sizeof(struct _tree_call_exp));
        if (!e)
                return NULL;

        tree_set_call_lhs(e, lhs);
        objgroup_init_ex(&_tree_get_call(e)->_args, tree_get_context_allocator(context));
        return e;
}

extern void tree_set_call_args(tree_exp* self, objgroup* args)
{
        objgroup* this_args = &_tree_get_call(self)->_args;
        objgroup_dispose(this_args);
        objgroup_move(this_args, args);
}

extern void tree_add_call_arg(tree_exp* self, tree_exp* arg)
{
        objgroup_push_back(&_tree_get_call(self)->_args, arg);
}

extern tree_exp* tree_new_subscript_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_exp*       lhs,
        tree_exp*       rhs)
{
        tree_exp* e = tree_new_exp(context, TEK_SUBSCRIPT, value_kind, type, loc,
                sizeof(struct _tree_subscript_exp));
        if (!e)
                return NULL;

        tree_set_subscript_lhs(e, lhs);
        tree_set_subscript_rhs(e, rhs);
        return e;
}

extern tree_exp* tree_new_conditional_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_exp*       condition,
        tree_exp*       lhs,
        tree_exp*       rhs)
{
        tree_exp* e = tree_new_exp(context, TEK_CONDITIONAL, value_kind, type, loc,
                sizeof(struct _tree_conditional_exp));
        if (!e)
                return NULL;

        tree_set_conditional_condition(e, condition);
        tree_set_conditional_lhs(e, lhs);
        tree_set_conditional_rhs(e, rhs);
        return e;
}

extern tree_exp* tree_new_integer_literal(
        tree_context* context, tree_type* type, tree_location loc, suint64 value)
{
        tree_exp* e = tree_new_exp(context, TEK_INTEGER_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_integer_literal_exp));
        if (!e)
                return NULL;

        tree_set_integer_literal(e, value);
        return e;
}

extern tree_exp* tree_new_character_literal(
        tree_context* context, tree_type* type, tree_location loc, int value)
{
        tree_exp* e = tree_new_exp(context, TEK_CHARACTER_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_character_literal_exp));
        if (!e)
                return NULL;

        tree_set_character_literal(e, value);
        return e;
}

extern tree_exp* tree_new_floating_literal(
        tree_context* context, tree_type* type, tree_location loc, float value)
{
        tree_exp* e = tree_new_exp(context, TEK_FLOATING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_floating_literal_exp));
        if (!e)
                return NULL;

        tree_set_floating_literal(e, value);
        return e;
}

extern tree_exp* tree_new_floating_lliteral(
        tree_context* context, tree_type* type, tree_location loc, ldouble value)
{
        tree_exp* e = tree_new_exp(context, TEK_FLOATING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_floating_literal_exp));
        if (!e)
                return NULL;

        tree_set_floating_lliteral(e, value);
        return e;
}

extern tree_exp* tree_new_string_literal(
        tree_context* context, tree_type* type, tree_location loc, tree_id ref)
{
        tree_exp* e = tree_new_exp(context, TEK_STRING_LITERAL, TVK_RVALUE, type, loc,
                sizeof(struct _tree_string_literal_exp));
        if (!e)
                return NULL;

        tree_set_string_literal(e, ref);
        return e;
}

extern tree_exp* tree_new_decl_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_decl*      decl)
{
        tree_exp* e = tree_new_exp(context, TEK_DECL, value_kind, type, loc, 
                sizeof(struct _tree_decl_exp));
        if (!e)
                return NULL;

        tree_set_decl_exp_entity(e, decl);
        return e;
}

extern tree_exp* tree_new_member_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_exp*       lhs,
        tree_decl*      decl,
        bool            is_arrow)
{
        tree_exp* e = tree_new_exp(context, TEK_MEMBER, value_kind, type, loc,
                sizeof(struct _tree_member_exp));
        if (!e)
                return NULL;

        tree_set_member_exp_decl(e, decl);
        tree_set_member_exp_arrow(e, is_arrow);
        tree_set_member_exp_lhs(e, lhs);
        return e;
}

extern tree_exp* tree_new_sizeof_exp(
        tree_context* context, tree_type* type, tree_location loc, void* rhs, bool is_unary)
{
        tree_exp* e = tree_new_exp(context, TEK_SIZEOF, TVK_RVALUE, type, loc,
                sizeof(struct _tree_sizeof_exp));
        if (!e)
                return NULL;

        if (is_unary)
                tree_set_sizeof_exp(e, rhs);
        else
                tree_set_sizeof_type(e, rhs);

        tree_set_sizeof_unary(e, is_unary);
        return e;
}

extern tree_exp* tree_new_paren_exp(
        tree_context*   context,
        tree_value_kind value_kind,
        tree_type*      type,
        tree_location   loc,
        tree_exp*       exp)
{
        tree_exp* e = tree_new_exp(context, TEK_PAREN, value_kind, type, loc,
                sizeof(struct _tree_paren_exp));
        if (!e)
                return NULL;
      
        tree_set_paren_exp(e, exp);
        return e;
}

extern tree_exp* tree_new_init_exp(tree_context* context, tree_location loc)
{
        tree_exp* e = tree_new_exp(context, TEK_INIT, TVK_RVALUE, NULL, loc,
                sizeof(struct _tree_init_exp));
        if (!e)
                return NULL;

        list_init(&_tree_get_init_exp(e)->_designations);
        return e;
}

extern void tree_add_init_designation(tree_exp* self, tree_designation* d)
{
        list_push_back(&_tree_get_init_exp(self)->_designations, &d->_node);
}

extern tree_designator* tree_new_member_designator(tree_context* context, tree_decl* member)
{
        tree_designator* d = tree_context_fast_allocate(context, sizeof(tree_designator));
        if (!d)
                return NULL;

        list_node_init(&_tree_get_designator(d)->_node);
        tree_set_designator_kind(d, TDK_DES_MEMBER);
        tree_set_member_designator_decl(d, member);
        return d;
}

extern tree_designator* tree_new_array_designator(
        tree_context* context, tree_type* eltype, tree_exp* index)
{
        tree_designator* d = tree_context_fast_allocate(context, sizeof(tree_designator));
        if (!d)
                return NULL;

        list_node_init(&_tree_get_designator(d)->_node);
        tree_set_designator_kind(d, TDK_DES_ARRAY);
        tree_set_array_designator_index(d, index);
        tree_set_array_designator_type(d, eltype);
        return d;
}

extern tree_designation* tree_new_designation(tree_context* context, tree_exp* initializer)
{
        tree_designation* d = tree_context_fast_allocate(context, sizeof(tree_designation));
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

extern bool tree_exp_is_literal(const tree_exp* self)
{
        switch (tree_get_exp_kind(self))
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

extern tree_exp* tree_ignore_impl_casts(tree_exp* self)
{
        while(tree_exp_is(self, TEK_IMPLICIT_CAST))
                self = tree_get_cast_exp(self);
        return self;
}

extern const tree_exp* tree_ignore_impl_ccasts(const tree_exp* self)
{
        while (tree_exp_is(self, TEK_IMPLICIT_CAST))
                self = tree_get_cast_exp(self);
        return self;
}

extern tree_exp* tree_ignore_paren_exps(tree_exp* self)
{
        while (tree_exp_is(self, TEK_PAREN))
                self = tree_get_paren_exp(self);
        return self;
}

extern const tree_exp* tree_ignore_paren_cexps(const tree_exp* self)
{
        while (tree_exp_is(self, TEK_PAREN))
                self = tree_get_paren_exp(self);
        return self;
}

extern tree_exp* tree_desugar_exp(tree_exp* self)
{
        tree_exp* ignored = self;
        while ((ignored = tree_ignore_paren_exps(tree_ignore_impl_casts(ignored))) != self)
                self = ignored;
        return ignored;
}

extern const tree_exp* tree_desugar_cexp(const tree_exp* self)
{
        const tree_exp* ignored = self;
        while ((ignored = tree_ignore_paren_cexps(tree_ignore_impl_ccasts(ignored))) != self)
                self = ignored;
        return ignored;
}

extern bool tree_exp_is_null_pointer_constant(const tree_exp* self)
{
        return false; // todo
}

extern bool tree_exp_designates_bitfield(const tree_exp* self)
{
        return false; // todo
}

