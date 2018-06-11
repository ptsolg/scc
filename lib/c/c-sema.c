#include "scc/c/c-sema.h"
#include "scc/c/c-parse-decl.h"
#include "c-misc.h"
#include "scc/c/c-errors.h"
#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-stmt.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-module.h"

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
        c_switch_stack_init_ex(&self->switch_stack, alloc);
        ptrvec_init_ex(&self->tm_info.non_atomic_gotos, alloc);
        strmap_init_ex(&self->tm_info.atomic_labels, alloc);
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
                "static unsigned __atomic_add_fetch_32_seq_cst(volatile unsigned* ptr, unsigned value);");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_XCHG_32_SEQ_CST, "__atomic_xchg_32_seq_cst",
                "static void __atomic_xchg_32_seq_cst(volatile unsigned* ptr, unsigned value);");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_SEQ_CST, "__atomic_fence_st_seq_cst",
                "static void __atomic_fence_st_seq_cst();");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_ACQ, "__atomic_fence_st_acq",
                "static void __atomic_fence_st_acq();");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_REL, "__atomic_fence_st_rel",
                "static void __atomic_fence_st_rel();");

        c_sema_init_builtin_function(self,
                TFBK_ATOMIC_FENCE_ST_ACQ_REL, "__atomic_fence_st_acq_rel",
                "static void __atomic_fence_st_acq_rel();");
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

        ptrvec_resize(&self->tm_info.non_atomic_gotos, 0);
        strmap_clear(&self->tm_info.atomic_labels);
}

extern void c_sema_push_scope(c_sema* self)
{
        c_sema_enter_decl_scope(self, tree_new_decl_scope(self->context, self->locals));
}

extern void c_sema_push_switch_stmt_info(c_sema* self, tree_stmt* switch_stmt)
{
        c_switch_stmt info;
        info.has_default_label = false;
        info.in_atomic_block = false;
        info.switch_stmt = switch_stmt;
        c_casemap_init_ex(&info.labels, c_context_get_allocator(self->ccontext));
        c_switch_stack_push(&self->switch_stack, info);
}

extern void c_sema_pop_switch_stmt_info(c_sema* self)
{
        c_casemap_dispose(&c_sema_get_switch_stmt_info(self)->labels);
        c_switch_stack_pop(&self->switch_stack);
}

extern void c_sema_set_switch_stmt_has_default_label(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->has_default_label = true;
}

extern void c_sema_set_switch_stmt_in_atomic_block(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->in_atomic_block = true;
}

extern c_switch_stmt* c_sema_get_switch_stmt_info(const c_sema* self)
{
        return c_switch_stack_last_p(&self->switch_stack);
}

extern bool c_sema_in_switch_stmt(const c_sema* self)
{
        return self->switch_stack.size != 0;
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

        c_casemap* labels = &c_sema_get_switch_stmt_info(self)->labels;
        for (uint32_t i = 1; ; i++)
        {
                c_casemap_entry* entry = c_casemap_lookup(labels, key);
                if (entry && int_cmp(val, tree_get_case_cvalue(entry->value)) == CR_EQ)
                        return false;
                else if (key != C_CASEMAP_EMPTY_KEY && key != C_CASEMAP_DELETED_KEY)
                        break;
                key += i;
        }

        c_casemap_insert(labels, key, label);
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