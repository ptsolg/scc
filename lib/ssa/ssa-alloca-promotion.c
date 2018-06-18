#include "scc/ssa/ssa-opt.h"
#include "scc/ssa/ssa-builder.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-context.h"

#define HTAB_FN(N) ssa_scope_##N
#define HTAB_TP    ssa_scope
#define HTAB_ETP   ssa_scope_entry
#define HTAB_KTP   ssa_id
#define HTAB_EK    -1
#define HTAB_DK    -2
#define HTAB_VTP   ssa_value*
#include "scc/core/htab-type.h"
#define SSA_SCOPE_FOREACH(PSCOPE, IT) \
        HTAB_FOREACH(PSCOPE, ssa_scope_entry, IT, ssa_scope_first_entry, ssa_scope_next_entry)

typedef struct
{
        ssa_value* def;
        int num_parents;
} ssa_parent_def;

#define HTAB_FN(N) ssa_parent_defs_##N
#define HTAB_TP    ssa_parent_defs
#define HTAB_ETP   ssa_parent_defs_entry
#define HTAB_KTP   ssa_id
#define HTAB_EK    -1
#define HTAB_DK    -2
#define HTAB_VTP   ssa_parent_def
#include "scc/core/htab-type.h"

#define SSA_PARENT_DEFS_FOREACH(PSCOPE, IT) \
        HTAB_FOREACH(PSCOPE, ssa_parent_defs_entry, IT, ssa_parent_defs_first_entry, ssa_parent_defs_next_entry)

typedef struct
{
        ssa_scope defs;
        ssa_scope phis;
} ssa_block_metadata;

static bool ssa_alloca_is_promotable(ssa_instr* alloca)
{
        ssa_value* var = ssa_get_instr_var(alloca);
        bool has_store = false;
        SSA_FOREACH_VALUE_USE(var, use, end)
        {
                ssa_instr* instr = ssa_get_value_use_instr(use);
                ssa_instr_kind k = ssa_get_instr_kind(instr);
                if (k == SIK_STORE)
                {
                        if (var != ssa_get_instr_operand_value(instr, 1))
                                return false;

                        has_store = true;
                }
                else if (k != SIK_LOAD || !has_store)
                        return false;
        }
        return true;
}

static void ssa_find_promotable_allocas(
        ssa_context* context, ssa_block* block, ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);

        ssa_block_metadata* md = ssa_allocate(context, sizeof(ssa_block_metadata));
        ssa_scope_init_ex(&md->defs, ssa_get_alloc(context));
        ssa_scope_init_ex(&md->phis, ssa_get_alloc(context));
        ssa_set_value_metadata(label, md);

        SSA_FOREACH_BLOCK_INSTR(block, instr)
        {
                if (ssa_get_instr_kind(instr) == SIK_ALLOCA)
                {
                        if (!ssa_alloca_is_promotable(instr))
                                continue;

                        ssa_value* var = ssa_get_instr_var(instr);
                        ssa_scope_insert(promotable_allocas, ssa_get_value_id(var), var);
                }
        }
}

static void ssa_promote_load(ssa_block* block,
        ssa_instr* instr, ssa_context* context, ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);
        ssa_block_metadata* md = ssa_get_value_metadata(label);

        ssa_value* source = ssa_get_instr_operand_value(instr, 0);
        ssa_id source_id = ssa_get_value_id(source);
        if (!ssa_scope_has(promotable_allocas, source_id))
                return;

        ssa_scope_entry* def = ssa_scope_lookup(&md->defs, source_id);
        ssa_value* val = def
                ? def->value
                : ssa_new_undef(context, tree_get_pointer_target(ssa_get_value_type(source)));

        ssa_replace_value_with(ssa_get_instr_var(instr), val);
        ssa_remove_instr(instr);
}

static void ssa_promote_allocas_in_block(ssa_context*, ssa_block*, ssa_scope*);

static void ssa_merge_parent_defs(ssa_context* context, ssa_block* block, ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);
        ssa_block_metadata* md = ssa_get_value_metadata(label);

        int num_parents = 0;
        ssa_parent_defs parent_defs;
        ssa_parent_defs_init_ex(&parent_defs, ssa_get_alloc(context));

        SSA_FOREACH_VALUE_USE(label, it, end)
        {
                ssa_instr* instr = ssa_get_value_use_instr(it);
                if (ssa_get_instr_kind(instr) != SIK_TERMINATOR)
                        continue;

                ssa_block* parent_block = ssa_get_instr_block(instr);
                ssa_value* parent_label = ssa_get_block_label(parent_block);
                if (ssa_get_value_id(parent_label) >= ssa_get_value_id(label))
                        continue;

                ssa_block_metadata* parent_md = ssa_get_value_metadata(parent_label);
                if (!parent_md)
                {
                        // todo: topological sort?
                        ssa_promote_allocas_in_block(context, parent_block, promotable_allocas);
                        parent_md = ssa_get_value_metadata(parent_label);
                }
        
                assert(parent_md);
                SSA_SCOPE_FOREACH(&parent_md->defs, it)
                {
                        ssa_parent_defs_entry* e = ssa_parent_defs_lookup(&parent_defs, it->key);
                        if (!e)
                        {
                                ssa_parent_def def = { it->value, 1 };
                                ssa_parent_defs_insert(&parent_defs, it->key, def);
                        }
                        else
                                e->value.num_parents++;
                }
                num_parents++;
        }

        SSA_PARENT_DEFS_FOREACH(&parent_defs, it)
        {
                if (it->value.num_parents != num_parents)
                        continue;

                ssa_builder b;
                ssa_init_builder(&b, context, ssa_get_block_instrs_begin(block));
                ssa_value* phi = ssa_build_phi(&b, ssa_get_value_type(it->value.def));
                ssa_scope_update(&md->phis, it->key, phi);
                ssa_scope_update(&md->defs, it->key, phi);
        }

        ssa_parent_defs_dispose(&parent_defs);
}

static void ssa_promote_allocas_in_block(
        ssa_context* context, ssa_block* block, ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);
        ssa_block_metadata* md = ssa_get_value_metadata(label);

        ssa_merge_parent_defs(context, block, promotable_allocas);

        SSA_FOREACH_BLOCK_INSTR_SAFE(block, instr, next)
        {
                ssa_instr_kind k = ssa_get_instr_kind(instr);
                if (k == SIK_LOAD)
                        ssa_promote_load(block, instr, context, promotable_allocas);
                else if (k == SIK_STORE)
                {
                        ssa_value* dest = ssa_get_instr_operand_value(instr, 1);
                        ssa_id dest_id = ssa_get_value_id(dest);
                        if (!ssa_scope_has(promotable_allocas, dest_id))
                                continue;

                        ssa_scope_update(&md->defs, dest_id, ssa_get_instr_operand_value(instr, 0));
                        ssa_remove_instr(instr);
                }
        }
}

static void ssa_update_phi_operands(ssa_context* context, ssa_block* block)
{
        ssa_value* label = ssa_get_block_label(block);
        ssa_block_metadata* md = ssa_get_value_metadata(label);

        ssa_instr* terminator = ssa_get_block_terminator(block);
        SSA_FOREACH_TERMINATOR_SUCCESSOR(terminator, it, end)
        {
                ssa_value* child_label = ssa_get_value_use_value(it);
                ssa_block_metadata* child_md = ssa_get_value_metadata(child_label);

                assert(child_md);
                SSA_SCOPE_FOREACH(&child_md->phis, it)
                {
                        ssa_scope_entry* e = ssa_scope_lookup(&md->defs, it->key);
                        if (!e)
                                continue;

                        ssa_instr* instr = ssa_get_var_instr(it->value);
                        if (ssa_get_instr_kind(instr) != SIK_PHI)
                                continue;

                        ssa_add_phi_operand(instr, context, e->value, label);
                }
        }
}

static void ssa_eliminate_unused_allocas_and_trivial_phis(ssa_block* block)
{
        SSA_FOREACH_BLOCK_INSTR_SAFE(block, instr, next)
        {
                ssa_instr_kind k = ssa_get_instr_kind(instr);
                if ((k == SIK_ALLOCA || k == SIK_PHI) && !ssa_value_is_used(ssa_get_instr_var(instr)))
                        ssa_remove_instr(instr);
                else if (k == SIK_PHI && ssa_get_instr_operands_size(instr) == 2)
                {
                        ssa_value* operand = ssa_get_instr_operand_value(instr, 0);
                        ssa_replace_value_with(ssa_get_instr_var(instr), operand);
                        ssa_remove_instr(instr);
                }
        }
}

extern void ssa_promote_allocas(const ssa_pass* pass)
{
        if (!ssa_function_has_body(pass->function))
                return;

        ssa_scope promotable_allocas;
        ssa_scope_init_ex(&promotable_allocas, ssa_get_alloc(pass->context));

        SSA_FOREACH_FUNCTION_BLOCK(pass->function, block)
                ssa_find_promotable_allocas(pass->context, block, &promotable_allocas);

        SSA_FOREACH_FUNCTION_BLOCK(pass->function, block)
                ssa_promote_allocas_in_block(pass->context, block, &promotable_allocas);

        SSA_FOREACH_FUNCTION_BLOCK(pass->function, block)
                ssa_update_phi_operands(pass->context, block);

        SSA_FOREACH_FUNCTION_BLOCK(pass->function, block)
        {
                ssa_eliminate_unused_allocas_and_trivial_phis(block);

                ssa_block_metadata* md = ssa_remove_value_metadata(ssa_get_block_label(block));
                ssa_scope_dispose(&md->defs);
                ssa_scope_dispose(&md->phis);
                ssa_deallocate(pass->context, md);
        }

        ssa_scope_dispose(&promotable_allocas);
}
