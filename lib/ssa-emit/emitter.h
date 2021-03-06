#ifndef SSA_EMITTER_H
#define SSA_EMITTER_H

#include "scc/ssa/builder.h"
#include "scc/core/vec.h"

typedef struct _ssa_module ssa_module;

typedef struct
{
        ssa_context* context;
        ssa_module* module;

        struct
        {
                ssa_value* read;
                ssa_value* read_word;
                ssa_value* write;
                ssa_value* write_word;
                ssa_value* start;
                ssa_value* end;
                ssa_value* abort;
                ssa_value* commit;
                ssa_value* commit_n;
                ssa_value* alloca;
                ssa_value* active;
                ssa_value* set_jmp;
                tree_type* word;
                size_t word_size;
        } tm_info;

        struct hashmap globals;
        struct ptrset* emitted_records;
} ssa_module_emitter;

extern bool ssa_record_is_emmited(ssa_module_emitter* self, const tree_decl* decl);
extern void ssa_set_record_emmited(ssa_module_emitter* self, const tree_decl* decl);

extern void ssa_set_global_decl(ssa_module_emitter* self, tree_decl* decl, ssa_value* val);
extern ssa_value* ssa_get_global_decl(ssa_module_emitter* self, const tree_decl* decl);

extern void ssa_emit_type(ssa_module_emitter* self, const tree_type* type);
extern bool ssa_emit_global_decl(ssa_module_emitter* self, tree_decl* decl);
extern ssa_module* ssa_finish_module(ssa_module_emitter* self);
extern ssa_const* ssa_emit_const_expr(ssa_module_emitter* self, const tree_expr* expr);

#define VEC ssa_scope_stack
#define VEC_T struct hashmap
#include "scc/core/vec.inc"

typedef struct
{
        ssa_module_emitter* module_emitter;
        ssa_context* context;
        ssa_block* block;
        ssa_value* function;
        ssa_instr* alloca_insertion_pos;
        ssa_builder builder;
        unsigned atomic_stmt_nesting;

        struct hashmap labels;
        struct ssa_scope_stack defs;

        struct vec continue_stack;
        struct vec break_stack;
        struct vec switch_stack;
} ssa_function_emitter;

extern void ssa_init_function_emitter(
        ssa_function_emitter* self, ssa_module_emitter* module_emitter, ssa_value* func);

extern void ssa_dispose_function_emitter(ssa_function_emitter* self);

extern void ssa_enter_block(ssa_function_emitter* self, ssa_block* block);
extern void ssa_emit_block(ssa_function_emitter* self, ssa_block* block);
extern void ssa_emit_current_block(ssa_function_emitter* self);
extern ssa_block* ssa_new_function_block(ssa_function_emitter* self);
extern bool ssa_current_block_is_terminated(const ssa_function_emitter* self);
extern void ssa_push_scope(ssa_function_emitter* self);
extern void ssa_pop_scope(ssa_function_emitter* self);
extern void ssa_set_def(ssa_function_emitter* self, const tree_decl* var, ssa_value* def);
extern ssa_value* ssa_get_def(ssa_function_emitter* self, const tree_decl* var);

extern ssa_block* ssa_get_block_for_label(ssa_function_emitter* self, const tree_decl* label);

extern void ssa_push_continue_dest(ssa_function_emitter* self, ssa_block* block);
extern void ssa_push_break_dest(ssa_function_emitter* self, ssa_block* block);
extern void ssa_push_switch_instr(ssa_function_emitter* self, ssa_instr* switch_instr);
extern void ssa_pop_continue_dest(ssa_function_emitter* self);
extern void ssa_pop_break_dest(ssa_function_emitter* self);
extern void ssa_pop_switch_instr(ssa_function_emitter* self);
extern ssa_block* ssa_get_continue_dest(ssa_function_emitter* self);
extern ssa_block* ssa_get_break_dest(ssa_function_emitter* self);
extern ssa_instr* ssa_get_switch_instr(ssa_function_emitter* self);

extern bool ssa_in_atomic_block(const ssa_function_emitter* self);

extern bool ssa_emit_local_decl(ssa_function_emitter* self, tree_decl* decl);

extern bool ssa_emit_cond_jmp(
        ssa_function_emitter* self, ssa_value* cond, ssa_block* ontrue, ssa_block* onfalse);
extern bool ssa_emit_jmp(ssa_function_emitter* self, ssa_block* dest);
extern bool ssa_emit_jmp_opt(ssa_function_emitter* self, ssa_block* dest);

extern bool ssa_emit_stmt(ssa_function_emitter* self, const tree_stmt* stmt);
extern bool ssa_emit_stmt_as_atomic(ssa_function_emitter* self, const tree_stmt* stmt);

extern ssa_value* ssa_emit_alloca(ssa_function_emitter* self, tree_type* type);
extern ssa_value* ssa_emit_load(ssa_function_emitter* self, ssa_value* what);
extern ssa_value* ssa_emit_store(ssa_function_emitter* self, ssa_value* what, ssa_value* where);
extern ssa_value* ssa_emit_expr(ssa_function_emitter* self, const tree_expr* expr);
extern ssa_value* ssa_emit_expr_as_condition(ssa_function_emitter* self, const tree_expr* expr);

extern bool ssa_emit_local_var_initializer(ssa_function_emitter* self, ssa_value* var, const tree_expr* init);

#endif
