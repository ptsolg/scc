#include "scc/c/c-sema.h"
#include "scc/c/c-info.h"
#include "scc/c/c-errors.h"
#include "scc/scl/bit-utils.h"

#define DSEQ_VALUE_TYPE cswitch_stmt_info
#define DSEQ_TYPE cswitch_stack
#define DSEQ_INIT cswitch_stack_init
#define DSEQ_INIT_ALLOC cswitch_stack_init_alloc
#define DSEQ_DISPOSE cswitch_stack_dispose
#define DSEQ_GET_SIZE cswitch_stack_size
#define DSEQ_GET_CAPACITY cswitch_stack_capacity
#define DSEQ_GET_ALLOCATOR cswitch_stack_allocator
#define DSEQ_RESERVE cswitch_stack_reserve
#define DSEQ_RESIZE cswitch_stack_resize
#define DSEQ_GET_BEGIN cswitch_stack_begin
#define DSEQ_GET_END cswitch_stack_end
#define DSEQ_GET cswitch_stack_get
#define DSEQ_SET cswitch_stack_set
#define DSEQ_APPEND cswitch_stack_append

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

#define HTAB_TYPE ccase_label_map
#define HTAB_IMPL_FN_GENERATOR(NAME) _ccase_label_map_##NAME
#define HTAB_KEY_TYPE suint32
#define HTAB_DELETED_KEY CCASE_LABEL_MAP_EMPTY_KEY
#define HTAB_EMPTY_KEY CCASE_LABEL_MAP_DELETED_KEY
#define HTAB_VALUE_TYPE void*
#define HTAB_INIT ccase_label_map_init
#define HTAB_INIT_ALLOC ccase_label_map_init_alloc
#define HTAB_DISPOSE ccase_label_map_dispose
#define HTAB_GET_SIZE ccase_label_map_size
#define HTAB_GET_ALLOCATOR ccase_label_map_alloc
#define HTAB_RESERVE ccase_label_map_reserve
#define HTAB_CLEAR ccase_label_map_clear
#define HTAB_GROW ccase_label_map_grow
#define HTAB_INSERT ccase_label_map_insert
#define HTAB_FIND ccase_label_map_find

#define HTAB_ITERATOR_TYPE ccase_label_map_iter
#define HTAB_ITERATOR_GET_KEY ccase_label_map_iter_key
#define HTAB_ITERATOR_ADVANCE ccase_label_map_iter_advance
#define HTAB_ITERATOR_INIT ccase_label_map_iter_init
#define HTAB_ITERATOR_CREATE ccase_label_map_iter_create
#define HTAB_ITERATOR_IS_VALID ccase_label_map_iter_valid
#define HTAB_ITERATOR_GET_VALUE ccase_label_map_iter_value

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

extern void csema_init(csema* self, ccontext* context, clogger* logger)
{
        self->ccontext = context;
        self->context = cget_tree(context);
        self->module = NULL;
        self->globals = NULL;
        self->target = NULL;
        self->locals = NULL;
        self->function = NULL;
        self->labels = NULL;
        self->scope = NULL;
        self->logger = logger;
        cswitch_stack_init_alloc(&self->switch_stack, cget_alloc(self->ccontext));
}

extern void csema_dispose(csema* self)
{
        // todo
}

extern void csema_enter_module(csema* self, tree_module* module)
{
        self->module = module;
        self->globals = tree_get_module_globals(module);
        self->target = tree_get_module_target(module);
        self->locals = self->globals;
}

extern void csema_enter_scope(csema* self, tree_scope* scope)
{
        S_ASSERT(tree_get_scope_parent(scope) == self->scope
                && "scopes should be connected.");
        self->scope = scope;
        csema_enter_decl_scope(self, tree_get_scope_decls(scope));
}

extern void csema_exit_scope(csema* self)
{
        S_ASSERT(self->scope);
        self->scope = tree_get_scope_parent(self->scope);
        csema_exit_decl_scope(self);
}

extern void csema_enter_decl_scope(csema* self, tree_decl_scope* scope)
{
        S_ASSERT(tree_get_decl_scope_parent(scope) == self->locals
                && "scopes should be connected.");
        self->locals = scope;
}

extern void csema_exit_decl_scope(csema* self)
{
        tree_decl_scope* parent = tree_get_decl_scope_parent(self->locals);
        S_ASSERT(parent);
        self->locals = parent;
}

extern void csema_enter_function(csema* self, tree_decl* func)
{
        self->labels = tree_get_function_labels(func);
        self->function = func;
        csema_enter_decl_scope(self, tree_get_function_params(func));
}

extern void csema_exit_function(csema* self)
{
        self->labels = NULL;
        self->function = NULL;
        csema_exit_decl_scope(self);
}

extern void csema_push_scope(csema* self)
{
        csema_enter_decl_scope(self, tree_new_decl_scope(self->context, self->locals));
}

extern void csema_push_switch_stmt_info(csema* self, tree_stmt* switch_stmt)
{
        cswitch_stmt_info info;
        info.has_default_label = false;
        info.switch_stmt = switch_stmt;
        ccase_label_map_init_alloc(&info.labels, cget_alloc(self->ccontext));
        cswitch_stack_append(&self->switch_stack, info);
}

extern void csema_pop_switch_stmt_info(csema* self)
{
        ssize size = cswitch_stack_size(&self->switch_stack);
        S_ASSERT(size);
        ccase_label_map_dispose(&csema_get_switch_stmt_info(self)->labels);
        cswitch_stack_resize(&self->switch_stack, size - 1);
}

extern void csema_set_switch_stmt_has_default_label(csema* self)
{
        csema_get_switch_stmt_info(self)->has_default_label = true;
}

extern cswitch_stmt_info* csema_get_switch_stmt_info(const csema* self)
{
        ssize size = cswitch_stack_size(&self->switch_stack);
        S_ASSERT(size);
        return cswitch_stack_begin(&self->switch_stack) + size - 1;
}

extern bool csema_in_switch_stmt(const csema* self)
{
        return cswitch_stack_size(&self->switch_stack) != 0;
}

extern bool csema_switch_stmt_has_default_label(const csema* self)
{
        return csema_get_switch_stmt_info(self)->has_default_label;
}

extern bool csema_switch_stmt_register_case_label(const csema* self, tree_stmt* label)
{
        const int_value* val = tree_get_case_cvalue(label);
        suint32 key = int_get_u32(val);

        ccase_label_map* labels = &csema_get_switch_stmt_info(self)->labels;
        for (suint32 i = 1; ; i++)
        {
                ccase_label_map_iter res;
                if (ccase_label_map_find(labels, key, &res)
                        && int_cmp(val, tree_get_case_cvalue(*ccase_label_map_iter_value(&res))) == CR_EQ)
                {
                        return false;
                }
                else if (key != CCASE_LABEL_MAP_EMPTY_KEY && key != CCASE_LABEL_MAP_DELETED_KEY)
                        break;
                key += i;
        }

        ccase_label_map_insert(labels, key, label);
        return true;
}

extern bool csema_at_file_scope(const csema* self)
{
        return self->locals == self->globals;
}

extern bool csema_at_block_scope(const csema* self)
{
        return (bool)self->function;
}