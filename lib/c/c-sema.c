#include "scc/c/c-sema.h"
#include "scc/c/c-info.h"
#include "scc/c/c-errors.h"
#include "scc/scl/bit-utils.h"

#define DSEQ_VALUE_TYPE c_switch_stmt_info
#define DSEQ_TYPE c_switch_stack
#define DSEQ_INIT c_switch_stack_init
#define DSEQ_INIT_ALLOC c_switch_stack_init_alloc
#define DSEQ_DISPOSE c_switch_stack_dispose
#define DSEQ_GET_SIZE c_switch_stack_size
#define DSEQ_GET_CAPACITY c_switch_stack_capacity
#define DSEQ_GET_ALLOCATOR c_switch_stack_allocator
#define DSEQ_RESERVE c_switch_stack_reserve
#define DSEQ_RESIZE c_switch_stack_resize
#define DSEQ_GET_BEGIN c_switch_stack_begin
#define DSEQ_GET_END c_switch_stack_end
#define DSEQ_GET c_switch_stack_get
#define DSEQ_SET c_switch_stack_set
#define DSEQ_APPEND c_switch_stack_append

#include "scc/scl/dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

#define CCASE_LABEL_MAP_EMPTY_KEY ((suint32)-1)
#define CCASE_LABEL_MAP_DELETED_KEY ((suint32)-2)

#define HTAB_TYPE c_case_label_map
#define HTAB_IMPL_FN_GENERATOR(NAME) _c_case_label_map_##NAME
#define HTAB_KEY_TYPE suint32
#define HTAB_DELETED_KEY CCASE_LABEL_MAP_EMPTY_KEY
#define HTAB_EMPTY_KEY CCASE_LABEL_MAP_DELETED_KEY
#define HTAB_VALUE_TYPE void*
#define HTAB_INIT c_case_label_map_init
#define HTAB_INIT_ALLOC c_case_label_map_init_alloc
#define HTAB_DISPOSE c_case_label_map_dispose
#define HTAB_GET_SIZE c_case_label_map_size
#define HTAB_GET_ALLOCATOR c_case_label_map_alloc
#define HTAB_RESERVE c_case_label_map_reserve
#define HTAB_CLEAR c_case_label_map_clear
#define HTAB_GROW c_case_label_map_grow
#define HTAB_INSERT c_case_label_map_insert
#define HTAB_FIND c_case_label_map_find

#define HTAB_ITERATOR_TYPE c_case_label_map_iter
#define HTAB_ITERATOR_GET_KEY c_case_label_map_iter_key
#define HTAB_ITERATOR_ADVANCE c_case_label_map_iter_advance
#define HTAB_ITERATOR_INIT c_case_label_map_iter_init
#define HTAB_ITERATOR_CREATE c_case_label_map_iter_create
#define HTAB_ITERATOR_IS_VALID c_case_label_map_iter_valid
#define HTAB_ITERATOR_GET_VALUE c_case_label_map_iter_value

#include "scc/scl/htab.h"

#undef HTAB_TYPE
#undef HTAB_IMPL_FN_GENERATOR
#undef HTAB_KEY_TYPE 
#undef HTAB_DELETED_KEY
#undef HTAB_EMPTY_KEY
#undef HTAB_VALUE_TYPE
#undef HTAB_INIT
#undef HTAB_INIT_ALLOC
#undef HTAB_DISPOSE
#undef HTAB_GET_SIZE
#undef HTAB_GET_ALLOCATOR
#undef HTAB_RESERVE
#undef HTAB_CLEAR
#undef HTAB_GROW
#undef HTAB_INSERT
#undef HTAB_FIND
#undef HTAB_ITERATOR_TYPE
#undef HTAB_ITERATOR_GET_KEY
#undef HTAB_ITERATOR_ADVANCE
#undef HTAB_ITERATOR_INIT
#undef HTAB_ITERATOR_CREATE
#undef HTAB_ITERATOR_IS_VALID
#undef HTAB_ITERATOR_GET_VALUE

extern void c_sema_init(c_sema* self, c_context* context, c_logger* logger)
{
        self->ccontext = context;
        self->context = c_context_get_tree_context(context);
        self->module = NULL;
        self->globals = NULL;
        self->target = NULL;
        self->locals = NULL;
        self->function = NULL;
        self->labels = NULL;
        self->scope = NULL;
        self->logger = logger;
        c_switch_stack_init_alloc(&self->switch_stack, c_context_get_allocator(self->ccontext));
}

extern void c_sema_dispose(c_sema* self)
{
        // todo
}

extern void c_sema_enter_module(c_sema* self, tree_module* module)
{
        self->module = module;
        self->globals = tree_get_module_globals(module);
        self->target = tree_get_module_target(module);
        self->locals = self->globals;
}

extern void c_sema_enter_scope(c_sema* self, tree_scope* scope)
{
        S_ASSERT(tree_get_scope_parent(scope) == self->scope
                && "scopes should be connected.");
        self->scope = scope;
        c_sema_enter_decl_scope(self, tree_get_scope_decls(scope));
}

extern void c_sema_exit_scope(c_sema* self)
{
        S_ASSERT(self->scope);
        self->scope = tree_get_scope_parent(self->scope);
        c_sema_exit_decl_scope(self);
}

extern void c_sema_enter_decl_scope(c_sema* self, tree_decl_scope* scope)
{
        S_ASSERT(tree_get_decl_scope_parent(scope) == self->locals
                && "scopes should be connected.");
        self->locals = scope;
}

extern void c_sema_exit_decl_scope(c_sema* self)
{
        tree_decl_scope* parent = tree_get_decl_scope_parent(self->locals);
        S_ASSERT(parent);
        self->locals = parent;
}

extern void c_sema_enter_function(c_sema* self, tree_decl* func)
{
        self->labels = tree_get_function_labels(func);
        self->function = func;
        c_sema_enter_decl_scope(self, tree_get_function_params(func));
}

extern void c_sema_exit_function(c_sema* self)
{
        self->labels = NULL;
        self->function = NULL;
        c_sema_exit_decl_scope(self);
}

extern void c_sema_push_scope(c_sema* self)
{
        c_sema_enter_decl_scope(self, tree_new_decl_scope(self->context, self->locals));
}

extern void c_sema_push_switch_stmt_info(c_sema* self, tree_stmt* switch_stmt)
{
        c_switch_stmt_info info;
        info.has_default_label = false;
        info.switch_stmt = switch_stmt;
        c_case_label_map_init_alloc(&info.labels, c_context_get_allocator(self->ccontext));
        c_switch_stack_append(&self->switch_stack, info);
}

extern void c_sema_pop_switch_stmt_info(c_sema* self)
{
        ssize size = c_switch_stack_size(&self->switch_stack);
        S_ASSERT(size);
        c_case_label_map_dispose(&c_sema_get_switch_stmt_info(self)->labels);
        c_switch_stack_resize(&self->switch_stack, size - 1);
}

extern void c_sema_set_switch_stmt_has_default_label(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->has_default_label = true;
}

extern c_switch_stmt_info* c_sema_get_switch_stmt_info(const c_sema* self)
{
        ssize size = c_switch_stack_size(&self->switch_stack);
        S_ASSERT(size);
        return c_switch_stack_begin(&self->switch_stack) + size - 1;
}

extern bool c_sema_in_switch_stmt(const c_sema* self)
{
        return c_switch_stack_size(&self->switch_stack) != 0;
}

extern bool c_sema_switch_stmt_has_default_label(const c_sema* self)
{
        return c_sema_get_switch_stmt_info(self)->has_default_label;
}

extern bool c_sema_switch_stmt_register_case_label(const c_sema* self, tree_stmt* label)
{
        const int_value* val = tree_get_case_cvalue(label);
        suint32 key = int_get_u32(val);

        c_case_label_map* labels = &c_sema_get_switch_stmt_info(self)->labels;
        for (suint32 i = 1; ; i++)
        {
                c_case_label_map_iter res;
                if (c_case_label_map_find(labels, key, &res)
                        && int_cmp(val, tree_get_case_cvalue(*c_case_label_map_iter_value(&res))) == CR_EQ)
                {
                        return false;
                }
                else if (key != CCASE_LABEL_MAP_EMPTY_KEY && key != CCASE_LABEL_MAP_DELETED_KEY)
                        break;
                key += i;
        }

        c_case_label_map_insert(labels, key, label);
        return true;
}

extern bool c_sema_at_file_scope(const c_sema* self)
{
        return self->locals == self->globals;
}

extern bool c_sema_at_block_scope(const c_sema* self)
{
        return (bool)self->function;
}