#include "scc/tree/tree-type.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-target.h"

extern tree_type* tree_new_modified_type(tree_context* context, tree_type* type)
{
        if (tree_type_is_modified(type))
                type = tree_get_modified_type(type);

        tree_type* mt = tree_allocate_node(context, sizeof(struct _tree_modified_type));
        if (!mt)
                return NULL;

        mt->modified.quals = TTQ_UNQUALIFIED;
        mt->modified.transaction_safe = false;
        mt->modified.type = type;
        mt = (tree_type*)((size_t)mt | _TREE_MODIFIED_TYPE_BIT);
        return mt;
}

extern tree_type* tree_new_qualified_type(
        tree_context* context, tree_type* type, tree_type_quals quals)
{
        tree_type* t = tree_new_modified_type(context, type);
        if (!t)
                return NULL;

        tree_set_type_quals(t, quals);
        return t;
}

extern tree_type* tree_new_type(tree_context* context, tree_type_kind kind, size_t size)
{
        tree_type* t = tree_allocate_node(context, size);
        if (!t)
                return NULL;

        tree_set_type_kind(t, kind);
        return t;
}

extern tree_type* tree_new_chain_type(
        tree_context* context, tree_type_kind kind, tree_type* next, size_t size)
{
        tree_type* t = tree_new_type(context, kind, size);
        if (!t)
                return NULL;

        tree_set_chain_type_next(t, next);
        return t;
}

extern void tree_init_builtin_type(tree_type* self, tree_builtin_type_kind kind)
{
        tree_set_type_kind(self, TTK_BUILTIN);
        tree_set_builtin_type_kind(self, kind);
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
        bool x32 = tree_target_is(context->target, TTAK_X86_32);
        return tree_new_builtin_type(context, x32 ? TBTK_UINT32 : TBTK_UINT64);
}

extern tree_type* tree_new_func_type(tree_context* context, tree_type* restype)
{
        tree_type* t = tree_new_chain_type(context, TTK_FUNCTION, restype, 
                sizeof(struct _tree_func_type));
        if (!t)
                return NULL;

        tree_set_func_type_vararg(t, false);
        tree_set_func_type_cc(t, TCC_DEFAULT);
        tree_init_array(&_tree_func_type(t)->params);
        return t;
}

extern errcode tree_add_func_type_param(tree_type* self, tree_context* context, tree_type* param)
{
        return tree_array_append_ptr(context, &_tree_func_type(self)->params, param);
}

extern void tree_init_array_type(tree_type* self, tree_array_kind kind, tree_type* eltype)
{
        tree_set_type_kind(self, TTK_ARRAY);
        tree_set_array_eltype(self, eltype);
        tree_set_array_kind(self, kind);
}

extern tree_type* tree_new_array_type(
        tree_context* context, tree_array_kind kind, tree_type* eltype)
{
        tree_type* t = tree_new_chain_type(context, TTK_ARRAY, eltype, sizeof(struct _tree_array_type));
        if (!t)
                return NULL;

        tree_set_array_kind(t, kind);
        return t;
}

extern void  tree_init_incomplete_array_type(tree_type* self, tree_type* eltype)
{
        tree_init_array_type(self, TAK_INCOMPLETE, eltype);
}

extern tree_type* tree_new_incomplete_array_type(tree_context* context, tree_type* eltype)
{
        return tree_new_array_type(context, TAK_INCOMPLETE, eltype);
}

extern void tree_init_constant_array_type(
        tree_type* self, tree_type* eltype, tree_expr* size_expr, const int_value* size_value)
{
        tree_init_array_type(self, TAK_CONSTANT, eltype);
        tree_set_array_size_expr(self, size_expr);
        tree_set_array_size_value(self, size_value);
}

extern tree_type* tree_new_constant_array_type(
        tree_context* context,
        tree_type* eltype,
        tree_expr* size_expr,
        const int_value* size_value)
{
        tree_type* t = tree_new_array_type(context, TAK_CONSTANT, eltype);
        if (!t)
                return NULL;

        tree_set_array_size_expr(t, size_expr);
        tree_set_array_size_value(t, size_value);
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

extern tree_type* tree_ignore_chain_types(tree_type* self)
{
        while (1)
        {
                self = tree_desugar_type(self);
                tree_type_kind k = tree_get_type_kind(self);
                if (k == TTK_POINTER || k == TTK_ARRAY || k == TTK_FUNCTION)
                        self = tree_get_chain_type_next(self);
                else
                        break;
        }
        return self;
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
        else if (tree_type_is_record(self) || tree_type_is_enumerated(self))
        {
                const tree_decl* entity = tree_get_decl_type_entity(self);
                return !tree_tag_decl_is_complete(entity);
        }
        
        return false;
}

extern bool tree_type_is_incomplete_or_object(const tree_type* self)
{
        return tree_type_is_incomplete(self) || tree_type_is_object(self);
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
                        UNREACHABLE();
                        return TBTK_INVALID;
        }
}

static bool tree_records_are_same(const tree_decl* a, const tree_decl* b)
{
        // todo
        return a == b;
}

static bool tree_enums_are_same(const tree_decl* a, const tree_decl* b)
{
        // todo
        return a == b;
}

static bool tree_decl_types_are_same(const tree_type* a, const tree_type* b)
{
        tree_decl* da = tree_get_decl_type_entity(a);
        tree_decl* db = tree_get_decl_type_entity(b);
        tree_decl_kind dk = tree_get_decl_kind(da);

        if (dk != tree_get_decl_kind(db))
                return false;

        assert(dk == TDK_RECORD || dk == TDK_ENUM);
        return dk == TDK_RECORD
                ? tree_records_are_same(da, db)
                : tree_enums_are_same(da, db);
}

static bool tree_array_types_are_same(const tree_type* a, const tree_type* b)
{
        if (tree_get_array_kind(a) != tree_get_array_kind(b))
                return false;
        if (tree_get_array_kind(a) == TAK_CONSTANT 
                && tree_get_array_size(a) != tree_get_array_size(b))
        {
                return false;
        }
        return true;
}

static tree_type_equality_kind tree_compare_function_type_params(const tree_type* a, const tree_type* b)
{
        if (tree_func_type_is_vararg(a) != tree_func_type_is_vararg(b))
                return TTEK_NEQ;

        size_t n = tree_get_func_type_params_size(a);
        if (n != tree_get_func_type_params_size(b))
                return TTEK_NEQ;

        tree_type_equality_kind k = TTEK_EQ;
        for (size_t i = 0; i < n; i++)
        {
                tree_type_equality_kind pk = tree_compare_types(
                        tree_get_func_type_param(a, i),
                        tree_get_func_type_param(b, i));
                if (pk == TTEK_NEQ)
                        return TTEK_NEQ;
                if (pk > k)
                        k = pk;
        }

        return k;
}

extern tree_type_equality_kind tree_compare_types(const tree_type* a, const tree_type* b)
{
        bool same_quals = true;
        while (1)
        {
                if (a == b)
                        return TTEK_EQ;

                same_quals &= tree_get_type_quals(a) == tree_get_type_quals(b);
                a = tree_desugar_ctype(a);
                b = tree_desugar_ctype(b);
                if (tree_get_modified_type_c(a) == tree_get_modified_type_c(b))
                        return same_quals ? TTEK_EQ : TTEK_DIFFERENT_QUALS;

                tree_type_kind k = tree_get_type_kind(a);
                if (k != tree_get_type_kind(b))
                        return TTEK_NEQ;

                switch (k)
                {
                        case TTK_BUILTIN:
                                if (tree_get_builtin_type_kind(a) != tree_get_builtin_type_kind(b))
                                        return TTEK_NEQ;
                                return same_quals ? TTEK_EQ : TTEK_DIFFERENT_QUALS;
                        case TTK_POINTER:
                                a = tree_get_pointer_target(a);
                                b = tree_get_pointer_target(b);
                                break;
                        case TTK_ARRAY:
                                if (!tree_array_types_are_same(a, b))
                                        return TTEK_NEQ;
                                a = tree_get_array_eltype(a);
                                b = tree_get_array_eltype(b);
                                break;
                        case TTK_FUNCTION:
                        {
                                if (tree_func_type_is_transaction_safe(a)
                                        != tree_func_type_is_transaction_safe(b))
                                {
                                        return TTEK_DIFFERENT_ATTRIBS;
                                }
                                tree_type_equality_kind ek = tree_compare_function_type_params(a, b);
                                if (ek == TTEK_NEQ)
                                        return TTEK_NEQ;
                                if (ek == TTEK_DIFFERENT_QUALS)
                                        same_quals = false;
                                a = tree_get_func_type_result(a);
                                b = tree_get_func_type_result(b);
                                break;
                        }
                        case TTK_DECL:
                                if (!tree_decl_types_are_same(a, b))
                                        return TTEK_NEQ;
                                return same_quals ? TTEK_EQ : TTEK_DIFFERENT_QUALS;

                        default:
                                assert(0 && "Invalid type");
                                return TTEK_NEQ;
                }
        }
}