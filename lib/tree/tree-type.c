#include "scc/tree/tree-type.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-target.h"

extern tree_type* tree_new_type(tree_context* context, tree_type_kind kind, ssize size)
{
        tree_type* t = tree_allocate(context, size);
        if (!t)
                return NULL;

        tree_set_type_kind(t, kind);
        return t;
}

extern tree_type* tree_new_chain_type(
        tree_context* context, tree_type_kind kind, tree_type* next, ssize size)
{
        tree_type* t = tree_new_type(context, kind, size);
        if (!t)
                return NULL;

        tree_set_chain_type_next(t, next);
        return t;
}

extern tree_type* tree_new_builtin_type(tree_context* context, tree_builtin_type_kind kind)
{
        tree_type* t = tree_new_type(context, TTK_BUILTIN, sizeof(struct _tree_builtin_type));
        if (!t)
                return NULL;
        
        tree_set_builtin_type_kind(t, kind);
        return t;
}

extern tree_type* tree_new_size_type(tree_context* context)
{
        bool x32 = tree_target_is(tree_get_target(context), TTARGET_X32);
        return tree_new_builtin_type(context, x32 ? TBTK_UINT32 : TBTK_UINT64);
}

extern tree_type* tree_new_function_type(tree_context* context, tree_type* restype)
{
        tree_type* t = tree_new_chain_type(context, TTK_FUNCTION, restype, 
                sizeof(struct _tree_function_type));
        if (!t)
                return NULL;

        tree_set_function_type_vararg(t, false);
        dseq_init_ex_ptr(&_tree_get_function_type(t)->_args,
                tree_get_allocator(context));
        return t;
}

extern void tree_set_function_type_params(tree_type* self, dseq* params)
{
        dseq* this_group = &_tree_get_function_type(self)->_args;
        dseq_dispose(this_group);
        dseq_move(this_group, params);
}

extern void tree_add_function_type_param(tree_type* self, tree_type* param)
{
        dseq_append_ptr(&_tree_get_function_type(self)->_args, param);
}

extern tree_type* tree_new_array_type_ex(
        tree_context* context, tree_array_kind kind, tree_type* eltype, ssize size)
{
        tree_type* t = tree_new_chain_type(context, TTK_ARRAY, eltype, size);
        if (!t)
                return NULL;

        tree_set_array_eltype(t, eltype);
        tree_set_array_kind(t, kind);
        return t;
}

extern tree_type* tree_new_array_type(
        tree_context* context, tree_array_kind kind, tree_type* eltype)
{
        return tree_new_array_type_ex(context, kind, eltype, sizeof(struct _tree_array_type));
}

extern tree_type* tree_new_incomplete_array_type(tree_context* context, tree_type* eltype)
{
        return tree_new_array_type(context, TAK_INCOMPLETE, eltype);
}

extern tree_type* tree_new_constant_array_type(
        tree_context* context,
        tree_type* eltype,
        tree_expr* size_expr,
        const int_value* size_value)
{
        tree_type* t = tree_new_array_type_ex(context, TAK_CONSTANT, eltype,
                sizeof(struct _tree_constant_array_type));
        if (!t)
                return NULL;

        tree_set_constant_array_size_expr(t, size_expr);
        tree_set_constant_array_size_value(t, size_value);
        return t;
}

extern tree_type* tree_new_decl_type(tree_context* context, tree_decl* decl, bool referenced)
{
        tree_type* t = tree_new_type(context, TTK_DECL, sizeof(struct _tree_decl_type));
        if (!t)
                return NULL;

        tree_set_decl_type_entity(t, decl);
        tree_set_decl_type_referenced(t, referenced);
        return t;
}

extern tree_type* tree_new_pointer_type(tree_context* context, tree_type* target)
{
        return tree_new_chain_type(context, TTK_POINTER, target,
                sizeof(struct _tree_pointer_type));
}

extern tree_type* tree_new_paren_type(tree_context* context, tree_type* next)
{
        return tree_new_chain_type(context, TTK_PAREN, next,
                sizeof(struct _tree_pointer_type));
}

extern tree_type* tree_ignore_typedefs(tree_type* self)
{
        tree_type* it = self;
        while (tree_declared_type_is(it, TDK_TYPEDEF))
                it = tree_get_decl_type(tree_get_decl_type_entity(it));
        return it;
}

extern const tree_type* tree_ignore_ctypedefs(const tree_type* self)
{
        const tree_type* it = self;
        while (tree_declared_type_is(it, TDK_TYPEDEF))
                it = tree_get_decl_type(tree_get_decl_type_entity(it));
        return it;
}

extern tree_type* tree_ignore_paren_types(tree_type* self)
{
        while (tree_type_is(self, TTK_PAREN))
                self = tree_get_paren_type(self);
        return self;
}

extern const tree_type* tree_ignore_paren_ctypes(const tree_type* self)
{
        while (tree_type_is(self, TTK_PAREN))
                self = tree_get_paren_type(self);
        return self;
}

extern tree_type* tree_desugar_type(tree_type* self)
{
        return tree_ignore_typedefs(tree_ignore_paren_types(self));
}

extern const tree_type* tree_desugar_ctype(const tree_type* self)
{
        return tree_ignore_ctypedefs(tree_ignore_paren_ctypes(self));
}

extern bool tree_builtin_type_is(const tree_type* self, tree_builtin_type_kind k)
{
        if (tree_get_type_kind(self) != TTK_BUILTIN)
                return false;

        return tree_get_builtin_type_kind(self) == k;
}

extern bool tree_declared_type_is(const tree_type* self, tree_decl_kind k)
{
        if (tree_get_type_kind(self) != TTK_DECL)
                return false;

        return tree_get_decl_kind(tree_get_decl_type_entity(self)) == k;
}

extern bool tree_type_is_void(const tree_type* self)
{
        return tree_builtin_type_is(tree_desugar_ctype(self), TBTK_VOID);
}

extern bool tree_type_is_signed_integer(const tree_type* self)
{
        self = tree_desugar_ctype(self);
        if (tree_get_type_kind(self) != TTK_BUILTIN)
                return false;

        switch (tree_get_builtin_type_kind(self))
        {
                case TBTK_INT8:
                case TBTK_INT16:
                case TBTK_INT32:
                case TBTK_INT64:
                        return true;

                default:
                        return false;
        }
}

extern bool tree_type_is_unsigned_integer(const tree_type* self)
{
        self = tree_desugar_ctype(self);
        if (tree_get_type_kind(self) != TTK_BUILTIN)
                return false;

        switch (tree_get_builtin_type_kind(self))
        {
                case TBTK_UINT8: 
                case TBTK_UINT16: 
                case TBTK_UINT32:
                case TBTK_UINT64:
                        return true;

                default:
                        return false;
        }
}

extern bool tree_type_is_real_floating(const tree_type* self)
{
        self = tree_desugar_ctype(self);
        if (tree_get_type_kind(self) != TTK_BUILTIN)
                return false;

        switch (tree_get_builtin_type_kind(self))
        {
                case TBTK_FLOAT:
                case TBTK_DOUBLE:
                        return true;

                default:
                        return false;
        }
}

extern bool tree_type_is_complex_floating(const tree_type* self)
{
        return false;
}

extern bool tree_type_is_derived(const tree_type* self)
{
        switch (tree_get_type_kind(tree_desugar_ctype(self)))
        {
                case TTK_FUNCTION:
                case TTK_POINTER:
                case TTK_ARRAY:
                        return true;

                default:
                        return false;
        }
}

extern bool tree_type_is_enumerated(const tree_type* self)
{
        return tree_declared_type_is(tree_desugar_ctype(self), TDK_ENUM);
}

extern bool tree_type_is_object(const tree_type* self)
{
        return !tree_type_is(tree_desugar_ctype(self), TTK_FUNCTION);
}

extern bool tree_type_is_real(const tree_type* self)
{
        return tree_type_is_integer(self) || tree_type_is_real_floating(self);
}

extern bool tree_type_is_scalar(const tree_type* self)
{
        return tree_type_is_arithmetic(self) || tree_type_is_pointer(self);
}

extern bool tree_type_is_arithmetic(const tree_type* self)
{
        return tree_type_is_integer(self) || tree_type_is_floating(self);
}

extern bool tree_type_is_pointer(const tree_type* self)
{
        return tree_type_is(tree_desugar_ctype(self), TTK_POINTER);
}

extern bool tree_type_is_record(const tree_type* self)
{
        return tree_declared_type_is(tree_desugar_ctype(self), TDK_RECORD);
}

extern bool tree_type_is_array(const tree_type* self)
{
        return tree_type_is(tree_desugar_ctype(self), TTK_ARRAY);
}

extern bool tree_type_is_function_pointer(const tree_type* self)
{
        self = tree_desugar_ctype(self);
        if (tree_type_is(self, TTK_POINTER))
        {
                const tree_type* eltype = tree_desugar_ctype(tree_get_pointer_target(self));
                return tree_type_is(eltype, TTK_FUNCTION);
        }
        return false;
}

extern bool tree_type_is_object_pointer(const tree_type* self)
{
        self = tree_desugar_ctype(self);
        if (tree_type_is(self, TTK_POINTER))
        {
                const tree_type* eltype = tree_desugar_ctype(tree_get_pointer_target(self));
                return tree_type_is_object(eltype);
        }
        return false;
}

extern bool tree_type_is_void_pointer(const tree_type* self)
{
        self = tree_desugar_ctype(self);
        if (tree_type_is(self, TTK_POINTER))
        {
                const tree_type* eltype = tree_desugar_ctype(tree_get_pointer_target(self));
                return tree_type_is_void(eltype);
        }
        return false;
}

extern bool tree_type_is_incomplete(const tree_type* self)
{
        if (!self)
                return true;

        self = tree_desugar_ctype(self);
        if (tree_type_is_void(self))
                return true;
        else if (tree_type_is_array(self))
                return tree_array_is(self, TAK_INCOMPLETE);
        else if (tree_type_is_record(self))
        {
                const tree_decl* entity = tree_get_decl_type_entity(self);
                return !tree_record_is_complete(entity);
        }
        
        return false;
}

extern bool tree_types_are_same(const tree_type* a, const tree_type* b)
{
        while(1)
        {
                if (a == b)
                        return true;

                if (tree_get_type_quals(a) != tree_get_type_quals(b))
                        return false;

                a = tree_desugar_ctype(a);
                b = tree_desugar_ctype(b);
                tree_type_kind ak = tree_get_type_kind(a);
                tree_type_kind bk = tree_get_type_kind(b);
                if (ak != bk)
                        return false;

                if (ak == TTK_BUILTIN)
                        return tree_get_builtin_type_kind(a) == tree_get_builtin_type_kind(b);
                else if (ak == TTK_POINTER)
                {
                        a = tree_get_pointer_target(a);
                        b = tree_get_pointer_target(b);
                }
                else if (ak == TTK_ARRAY)
                {
                        // todo: compare size
                        a = tree_get_array_eltype(a);
                        b = tree_get_array_eltype(b);
                }
                else if (ak == TTK_FUNCTION)
                {
                        if (tree_function_type_is_vararg(a) != tree_function_type_is_vararg(b))
                                return false;

                        ssize n = tree_get_function_type_nparams(a);
                        if (n != tree_get_function_type_nparams(b))
                                return false;

                        for (ssize i = 0; i < n; i++)
                        {
                                tree_type* ap = tree_get_function_type_param(a, i);
                                tree_type* bp = tree_get_function_type_param(b, i);
                                if (!tree_types_are_same(ap, bp))
                                        return false;
                        }

                        a = tree_get_function_type_result(a);
                        b = tree_get_function_type_result(b);
                }
                else if (ak == TTK_DECL)
                        return tree_decls_are_same(
                                tree_get_decl_type_entity(a), tree_get_decl_type_entity(b));
                else
                        S_UNREACHABLE();
        }
}

extern tree_type* tree_get_type_next(const tree_type* self)
{
        tree_type_kind k = tree_get_type_kind(self);
        switch (tree_get_type_kind(self))
        {
                case TTK_POINTER:
                case TTK_ARRAY:
                case TTK_FUNCTION:
                case TTK_PAREN:
                        return tree_get_chain_type_next(self);

                default:
                        return NULL;
        }
}

extern tree_type* tree_new_qual_type(
        tree_context* context, tree_type_quals quals, tree_type* type)
{
        S_ASSERT(type);
        if (tree_type_is_qualified(type))
                type = tree_get_unqualified_type(type);
        
        tree_type* qt = tree_allocate(context, sizeof(struct _tree_qualified_type));
        if (!qt)
                return NULL;

        qt->_qtype._type = (union _tree_unqualified_type*)type;
        qt->_qtype._quals = quals;
        qt = (tree_type*)((ssize)qt | _TREE_QUAL_FLAG);
        return qt;
}

extern bool tree_type_is_integer(const tree_type* t)
{
        t = tree_desugar_ctype(t);
        if (tree_get_type_kind(t) != TTK_BUILTIN)
                return tree_declared_type_is(t, TDK_ENUM);

        switch (tree_get_builtin_type_kind(t))
        {
                case TBTK_INT8:
                case TBTK_UINT8:
                case TBTK_INT16:
                case TBTK_UINT16:
                case TBTK_INT32:
                case TBTK_UINT32:
                case TBTK_INT64:
                case TBTK_UINT64:
                        return true;

                default:
                        return false;
        }
}

extern bool tree_type_is_floating(const tree_type* t)
{
        t = tree_desugar_ctype(t);
        if (tree_get_type_kind(t) != TTK_BUILTIN)
                return false;

        switch (tree_get_builtin_type_kind(t))
        {
                case TBTK_FLOAT:
                case TBTK_DOUBLE:
                        return true;

                default:
                        return false;
        }
}

extern tree_builtin_type_kind tree_get_integer_counterpart(const tree_type* t)
{
        switch (tree_get_builtin_type_kind(t))
        {
                case TBTK_INT8: return TBTK_UINT8;
                case TBTK_UINT8: return TBTK_INT8;
                case TBTK_INT16: return TBTK_UINT16;
                case TBTK_UINT16: return TBTK_INT16;
                case TBTK_INT32: return TBTK_UINT32;
                case TBTK_UINT32: return TBTK_INT32;
                case TBTK_INT64: return TBTK_UINT64;
                case TBTK_UINT64: return TBTK_INT64;

                default:
                        S_UNREACHABLE();
                        return TBTK_INVALID;
        }
}
