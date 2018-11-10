#include "emitter.h"
#include "scc/ssa/context.h"
#include "scc/ssa/optimize.h"
#include "scc/ssa/module.h"
#include "scc/ssa/block.h"

extern bool ssa_record_is_emmited(ssa_module_emitter* self, const tree_decl* decl)
{
        return ptrset_has(&self->emitted_records, decl);
}

extern void ssa_set_record_emmited(ssa_module_emitter* self, const tree_decl* decl)
{
        assert(decl);
        ptrset_insert(&self->emitted_records, decl);
}

extern void ssa_set_global_decl(ssa_module_emitter* self, tree_decl* decl, ssa_value* val)
{
        assert(decl);
        strmap_insert(&self->globals, tree_get_decl_name(decl), val);
}

extern ssa_value* ssa_get_global_decl(ssa_module_emitter* self, const tree_decl* decl)
{
        strmap_entry* entry = strmap_lookup(&self->globals, tree_get_decl_name(decl));
        return entry ? entry->value : NULL;
}

static bool ssa_load_implicit_modules(
        ssa_module_emitter* self, const tree_module* module, const ssa_implicitl_modules* ext)
{
        if (!ext || !ext->tm)
                return true;

        tree_context* context = self->context->tree;
        enum
        {
                READ,
                READ_WORD,
                WRITE,
                WRITE_WORD,
                START,
                END,
                ABORT,
                COMMIT,
                COMMIT_N,
                ALLOCA,
                ACTIVE,
                WORD,
                SETJMP,
                SIZE,
        };

        struct
        {
                const char* name;
                tree_decl_kind kind;
        } decls_info[] =
        {
                { "_tm_read", TDK_FUNCTION },
                { "_tm_read_word", TDK_FUNCTION },
                { "_tm_write", TDK_FUNCTION },
                { "_tm_write_word", TDK_FUNCTION },
                { "_tm_start", TDK_FUNCTION },
                { "_tm_end", TDK_FUNCTION },
                { "_tm_abort", TDK_FUNCTION },
                { "_tm_commit", TDK_FUNCTION },
                { "_tm_commit_n", TDK_FUNCTION },
                { "_tm_alloca", TDK_FUNCTION },
                { "_tm_active", TDK_FUNCTION },
                { "_tm_word", TDK_TYPEDEF },

                { "_setjmp", TDK_FUNCTION },
        };

        tree_decl* decls[SIZE];

        // todo: merge modules?
        for (int i = 0; i < SIZE; i++)
        {
                if (!(decls[i] = tree_module_lookup_s(module, TLK_DECL, decls_info[i].name)))
                        if (!(decls[i] = tree_module_lookup_s(ext->tm, TLK_DECL, decls_info[i].name)))
                                return false;
                if (!tree_decl_is(decls[i], decls_info[i].kind))
                        return false;
                if (!ssa_emit_global_decl(self, decls[i]))
                        return false;
        }

        self->tm_info.word = tree_get_decl_type(decls[WORD]);
        self->tm_info.word_size = tree_get_sizeof(self->context->target, self->tm_info.word);
        self->tm_info.read = ssa_get_global_decl(self, decls[READ]);
        self->tm_info.read_word = ssa_get_global_decl(self, decls[READ_WORD]);
        self->tm_info.write = ssa_get_global_decl(self, decls[WRITE]);
        self->tm_info.write_word = ssa_get_global_decl(self, decls[WRITE_WORD]);
        self->tm_info.start = ssa_get_global_decl(self, decls[START]);
        self->tm_info.end = ssa_get_global_decl(self, decls[END]);
        self->tm_info.abort = ssa_get_global_decl(self, decls[ABORT]);
        self->tm_info.commit = ssa_get_global_decl(self, decls[COMMIT]);
        self->tm_info.commit_n = ssa_get_global_decl(self, decls[COMMIT_N]);
        self->tm_info.alloca = ssa_get_global_decl(self, decls[ALLOCA]);
        self->tm_info.active = ssa_get_global_decl(self, decls[ACTIVE]);
        self->tm_info.setjmp = ssa_get_global_decl(self, decls[SETJMP]);

        return true;
}

static void ssa_init_module_emitter(ssa_module_emitter* self, ssa_context* context)
{
        self->context = context;
        self->module = NULL;

        self->tm_info.read = NULL;
        self->tm_info.read_word = NULL;
        self->tm_info.write = NULL;
        self->tm_info.write_word = NULL;
        self->tm_info.start = NULL;
        self->tm_info.end = NULL;
        self->tm_info.abort = NULL;
        self->tm_info.commit = NULL;
        self->tm_info.commit_n = NULL;
        self->tm_info.alloca = NULL;
        self->tm_info.active = NULL;
        self->tm_info.setjmp = NULL;
        self->tm_info.word = NULL;
        self->tm_info.word_size = 0;

        strmap_init_ex(&self->globals, ssa_get_alloc(context));
        ptrset_init_ex(&self->emitted_records, ssa_get_alloc(context));
}

static void ssa_dispose_module_emitter(ssa_module_emitter* self)
{
        strmap_dispose(&self->globals);
        ptrset_dispose(&self->emitted_records);
}

extern ssa_module* ssa_emit_module(
        ssa_context* context,
        const tree_module* module,
        const ssa_optimizer_opts* opts,
        const ssa_implicitl_modules* ext)
{
        bool failed = true;
        ssa_module_emitter module_emitter;
        ssa_init_module_emitter(&module_emitter, context);

        module_emitter.module = ssa_new_module(context);

        if (!ssa_load_implicit_modules(&module_emitter, module, ext))
                goto cleanup;

        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_FOREACH_DECL_IN_SCOPE(globals, it)
                if (!ssa_emit_global_decl(&module_emitter, it))
                        goto cleanup;

        ssa_number_module_values(module_emitter.module);

        if (opts)
                ssa_optimize(context, module_emitter.module, opts);

        if (!ssa_finish_module(&module_emitter))
                goto cleanup;

        // todo: post-finish optimization?
        failed = false;
cleanup:
        ssa_dispose_module_emitter(&module_emitter);
        return failed ? NULL : module_emitter.module;
}

extern void ssa_init_function_emitter(
        ssa_function_emitter* self, ssa_module_emitter* module_emitter, ssa_value* func)
{
        self->module_emitter = module_emitter;
        self->context = module_emitter->context;
        self->atomic_stmt_nesting = 0;
        self->block = ssa_new_function_block(self);
        self->function = func;
        self->alloca_insertion_pos = ssa_get_block_instrs_end(self->block);
        ssa_init_builder(&self->builder, self->context, ssa_get_block_instrs_end(self->block));

        allocator* alloc = ssa_get_alloc(self->context);
        strmap_init_ex(&self->labels, alloc);
        ssa_scope_stack_init_ex(&self->defs, alloc);
        ptrvec_init_ex(&self->continue_stack, alloc);
        ptrvec_init_ex(&self->break_stack, alloc);
        ptrvec_init_ex(&self->switch_stack, alloc);
}

extern void ssa_dispose_function_emitter(ssa_function_emitter* self)
{
        strmap_dispose(&self->labels);
        ssa_scope_stack_dispose(&self->defs);
        ptrvec_dispose(&self->continue_stack);
        ptrvec_dispose(&self->break_stack);
        ptrvec_dispose(&self->switch_stack);
}

extern void ssa_enter_block(ssa_function_emitter* self, ssa_block* block)
{
        self->block = block;
        self->builder.pos = ssa_get_block_instrs_end(self->block);
}

extern void ssa_emit_block(ssa_function_emitter* self, ssa_block* block)
{
        ssa_add_function_block(self->function, block);
}

extern void ssa_emit_current_block(ssa_function_emitter* self)
{
        ssa_emit_block(self, self->block);
        self->block = NULL;
}

extern ssa_block* ssa_new_function_block(ssa_function_emitter* self)
{
        return ssa_new_block(self->context, ssa_in_atomic_block(self));
}

extern bool ssa_current_block_is_terminated(const ssa_function_emitter* self)
{
        assert(self->block);
        ssa_instr* terminator = ssa_get_block_terminator(self->block);
        return terminator && ssa_get_instr_kind(terminator) == SIK_TERMINATOR;
}

static inline strmap* ssa_get_last_scope(const ssa_function_emitter* self)
{
        return ssa_scope_stack_last_p(&self->defs);
}

extern void ssa_push_scope(ssa_function_emitter* self)
{
        ssa_scope_stack_push(&self->defs, strmap_create_ex(ssa_get_alloc(self->context)));
}

extern void ssa_pop_scope(ssa_function_emitter* self)
{
        strmap_dispose(ssa_get_last_scope(self));
        ssa_scope_stack_pop(&self->defs);
}

extern void ssa_set_def(ssa_function_emitter* self, const tree_decl* var, ssa_value* def)
{
        assert(def);
        strmap* last = ssa_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        assert(!strmap_has(last, id));
        strmap_insert(last, id, def);
}

extern ssa_value* ssa_get_def(ssa_function_emitter* self, const tree_decl* var)
{
        strmap* first = ssa_scope_stack_begin(&self->defs);
        strmap* it = ssa_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        while (it >= first)
        {
                strmap_entry* entry = strmap_lookup(it, id);
                if (entry)
                        return entry->value;
                it--;
        }
        return ssa_get_global_decl(self->module_emitter, var);
}

extern ssa_block* ssa_get_block_for_label(ssa_function_emitter* self, const tree_decl* label)
{
        tree_id id = tree_get_decl_name(label);
        strmap_entry* entry = strmap_lookup(&self->labels, id);
        if (entry)
                return entry->value;

        ssa_block* b = ssa_new_function_block(self);
        strmap_insert(&self->labels, id, b);
        return b;
}

extern void ssa_push_continue_dest(ssa_function_emitter* self, ssa_block* block)
{
        ptrvec_push(&self->continue_stack, block);
}

extern void ssa_push_break_dest(ssa_function_emitter* self, ssa_block* block)
{
        ptrvec_push(&self->break_stack, block);
}

extern void ssa_push_switch_instr(ssa_function_emitter* self, ssa_instr* switch_instr)
{
        ptrvec_push(&self->switch_stack, switch_instr);
}

extern void ssa_pop_continue_dest(ssa_function_emitter* self)
{
        ptrvec_pop(&self->continue_stack);
}

extern void ssa_pop_break_dest(ssa_function_emitter* self)
{
        ptrvec_pop(&self->break_stack);
}

extern void ssa_pop_switch_instr(ssa_function_emitter* self)
{
        ptrvec_pop(&self->switch_stack);
}

extern ssa_block* ssa_get_continue_dest(ssa_function_emitter* self)
{
        return ptrvec_last(&self->continue_stack);
}

extern ssa_block* ssa_get_break_dest(ssa_function_emitter* self)
{
        return ptrvec_last(&self->break_stack);
}

extern ssa_instr* ssa_get_switch_instr(ssa_function_emitter* self)
{
        return ptrvec_last(&self->switch_stack);
}

extern bool ssa_in_atomic_block(const ssa_function_emitter* self)
{
        return self->atomic_stmt_nesting != 0;
}