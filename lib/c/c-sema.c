#include "scc/c/c-sema.h"
#include "scc/c/c-parse-decl.h"
#include "c-misc.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-stmt.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-module.h"

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

#include "scc/core/dseq.h"

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

#define CCASE_LABEL_MAP_EMPTY_KEY ((uint32_t)-1)
#define CCASE_LABEL_MAP_DELETED_KEY ((uint32_t)-2)

#define HTAB_TYPE c_case_label_map
#define HTAB_IMPL_FN_GENERATOR(NAME) _c_case_label_map_##NAME
#define HTAB_KEY_TYPE uint32_t
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
#define HTAB_ERASE c_case_label_map_erase
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

#include "scc/core/htab.h"

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
#undef HTAB_ERASE
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
        self->context = context->tree;
        self->module = NULL;
        self->globals = NULL;
        self->target = NULL;
        self->locals = NULL;
        self->function = NULL;
        self->labels = NULL;
        self->scope = NULL;
        self->logger = logger;
        self->tm_info.atomic_stmt_nesting = 0;

        allocator* alloc = c_context_get_allocator(self->ccontext);
        c_switch_stack_init_alloc(&self->switch_stack, alloc);
        dseq_init_alloc(&self->tm_info.non_atomic_gotos, alloc);
        strmap_init_alloc(&self->tm_info.atomic_labels, alloc);
}

extern void c_sema_dispose(c_sema* self)
{
        // todo
}

static void c_sema_init_builtin_function(c_sema* self, tree_function_builtin_kind kind, const char* name, const char* decl)
{
        c_source* s = c_source_emulate(&self->ccontext->source_manager, name, decl);
        c_lexer lexer;
        c_lexer_init(&lexer, self->logger, self->ccontext);
        if (!s || EC_FAILED(c_lexer_enter_source_file(&lexer, s)))
                goto error;

        c_parser parser;
        c_parser_init(&parser, &lexer, self, self->logger);
        jmp_buf on_parser_error;
        c_parser_set_on_error(&parser, on_parser_error);
        if (setjmp(on_parser_error))
                goto error;
        
        c_parser_enter_token_stream(&parser);
        tree_decl* func = c_parse_decl(&parser);
        if (!func)
                goto error;

        tree_set_func_builtin_kind(func, kind);
        tree_set_decl_implicit(func, true);
        return;

error:
        assert(0 && "unexpected error");
}

static void c_sema_init_builtin_functions(c_sema* self)
{
        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_CMPXCHG_32_WEAK_SEQ_CST, "__atomic_cmpxchg_32_weak_seq_cst",
                "static int __atomic_cmpxchg_32_weak_seq_cst("
                        "volatile unsigned* ptr, unsigned expected, unsigned desired);");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_ADD_FETCH_32_SEQ_CST, "__atomic_add_fetch_32_seq_cst",
                "static void __atomic_add_fetch_32_seq_cst(unsigned* ptr, unsigned value);");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_XCHG_32_SEQ_CST, "__atomic_xchg_32_seq_cst",
                "static void __atomic_xchg_32_seq_cst(unsigned* ptr, unsigned value);");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_SEQ_CST, "__atomic_fence_st_seq_cst",
                "static void __atomic_fence_st_seq_cst();");
}

extern tree_module* c_sema_new_module(c_sema* self)
{
        self->module = tree_new_module(self->context);
        self->globals = tree_get_module_globals(self->module);
        self->target = tree_get_module_target(self->module);
        self->locals = self->globals;
        c_sema_init_builtin_functions(self);
        return self->module;
}

extern void c_sema_enter_scope(c_sema* self, tree_scope* scope)
{
        assert(tree_get_scope_parent(scope) == self->scope
                && "scopes should be connected.");
        self->scope = scope;
        c_sema_enter_decl_scope(self, tree_get_scope_decls(scope));
}

extern void c_sema_exit_scope(c_sema* self)
{
        assert(self->scope);
        self->scope = tree_get_scope_parent(self->scope);
        c_sema_exit_decl_scope(self);
}

extern void c_sema_enter_decl_scope(c_sema* self, tree_decl_scope* scope)
{
        assert(tree_get_decl_scope_parent(scope) == self->locals
                && "scopes should be connected.");
        self->locals = scope;
}

extern void c_sema_exit_decl_scope(c_sema* self)
{
        tree_decl_scope* parent = tree_get_decl_scope_parent(self->locals);
        assert(parent);
        self->locals = parent;
}

extern void c_sema_enter_function(c_sema* self, tree_decl* func)
{
        self->labels = tree_get_func_labels(func);
        self->function = func;
        c_sema_enter_decl_scope(self, tree_get_func_params(func));
}

extern void c_sema_exit_function(c_sema* self)
{
        self->labels = NULL;
        self->function = NULL;
        c_sema_exit_decl_scope(self);

        dseq_resize(&self->tm_info.non_atomic_gotos, 0);
        strmap_clear(&self->tm_info.atomic_labels);
}

extern void c_sema_push_scope(c_sema* self)
{
        c_sema_enter_decl_scope(self, tree_new_decl_scope(self->context, self->locals));
}

extern void c_sema_push_switch_stmt_info(c_sema* self, tree_stmt* switch_stmt)
{
        c_switch_stmt_info info;
        info.has_default_label = false;
        info.in_atomic_block = false;
        info.switch_stmt = switch_stmt;
        c_case_label_map_init_alloc(&info.labels, c_context_get_allocator(self->ccontext));
        c_switch_stack_append(&self->switch_stack, info);
}

extern void c_sema_pop_switch_stmt_info(c_sema* self)
{
        size_t size = c_switch_stack_size(&self->switch_stack);
        assert(size);
        c_case_label_map_dispose(&c_sema_get_switch_stmt_info(self)->labels);
        c_switch_stack_resize(&self->switch_stack, size - 1);
}

extern void c_sema_set_switch_stmt_has_default_label(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->has_default_label = true;
}

extern void c_sema_set_switch_stmt_in_atomic_block(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->in_atomic_block = true;
}

extern c_switch_stmt_info* c_sema_get_switch_stmt_info(const c_sema* self)
{
        size_t size = c_switch_stack_size(&self->switch_stack);
        assert(size);
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

extern bool c_sema_switch_stmt_in_atomic_block(const c_sema* self)
{
        return c_sema_get_switch_stmt_info(self)->in_atomic_block;
}

extern bool c_sema_switch_stmt_register_case_label(const c_sema* self, tree_stmt* label)
{
        const int_value* val = tree_get_case_cvalue(label);
        uint32_t key = int_get_u32(val);

        c_case_label_map* labels = &c_sema_get_switch_stmt_info(self)->labels;
        for (uint32_t i = 1; ; i++)
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

extern bool c_sema_in_atomic_block(const c_sema* self)
{
        return self->tm_info.atomic_stmt_nesting != 0;
}

extern bool c_sema_in_transaction_safe_function(const c_sema* self)
{
        return self->function
                && tree_func_type_is_transaction_safe(
                        tree_desugar_type(tree_get_decl_type(self->function)));
}

extern bool c_sema_in_transaction_safe_block(const c_sema* self)
{
        return c_sema_in_atomic_block(self) || c_sema_in_transaction_safe_function(self);
}