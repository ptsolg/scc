#include "scc/ssa-optimize/optimize.h"
#include "scc/ssa/builder.h"
#include "scc/ssa/block.h"
#include "scc/ssa/context.h"

#define HTAB ssa_scope
#define HTAB_K ssa_id
#define HTAB_K_EMPTY -1
#define HTAB_K_DEL -2
#define HTAB_V ssa_value*
#include "scc/core/htab.inc"
#define SSA_SCOPE_FOREACH(PSCOPE, IT)\
        for (struct ssa_scope_iter IT = ssa_scope_begin(PSCOPE);\
                IT.pos != IT.end; ssa_scope_next(&IT))

typedef struct
{
        ssa_value* def;
        int num_parents;
} ssa_parent_def;

#define HTAB ssa_parent_defs
#define HTAB_K ssa_id
#define HTAB_K_EMPTY -1
#define HTAB_K_DEL -2
#define HTAB_V ssa_parent_def
#include "scc/core/htab.inc"
#define SSA_PARENT_DEFS_FOREACH(PDEFS, IT)\
        for (struct ssa_parent_defs_iter IT = ssa_parent_defs_begin(PDEFS);\
                IT.pos != IT.end; ssa_parent_defs_next(&IT))

typedef struct
{
        struct ssa_scope defs;
        struct ssa_scope phis;
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
        ssa_context* context, ssa_block* block, struct ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);

        ssa_block_metadata* md = alloc(sizeof(ssa_block_metadata));
        ssa_scope_init(&md->defs);
        ssa_scope_init(&md->phis);
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
        ssa_instr* instr, ssa_context* context, struct ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);
        ssa_block_metadata* md = ssa_get_value_metadata(label);

        ssa_value* source = ssa_get_instr_operand_value(instr, 0);
        ssa_id source_id = ssa_get_value_id(source);
        if (!ssa_scope_has(promotable_allocas, source_id))
                return;

        struct ssa_scope_entry* def = ssa_scope_lookup(&md->defs, source_id);
        ssa_value* val = def
                ? def->value
                : ssa_new_undef(context, tree_get_pointer_target(ssa_get_value_type(source)));

        ssa_replace_value_with(ssa_get_instr_var(instr), val);
        ssa_remove_instr(instr);
}

static void ssa_promote_allocas_in_block(ssa_context*, ssa_block*, struct ssa_scope*);

static void ssa_merge_parent_defs(ssa_context* context, ssa_block* block, struct ssa_scope* promotable_allocas)
{
        ssa_value* label = ssa_get_block_label(block);
        ssa_block_metadata* md = ssa_get_value_metadata(label);

        int num_parents = 0;
        struct ssa_parent_defs parent_defs;
        ssa_parent_defs_init(&parent_defs);

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
                        struct ssa_parent_defs_entry* e = ssa_parent_defs_lookup(&parent_defs, it.pos->key);
                        if (!e)
                        {
                                ssa_parent_def def = { it.pos->value, 1 };
                                ssa_parent_defs_insert(&parent_defs, it.pos->key, def);
                        }
                        else
                                e->value.num_parents++;
                }
                num_parents++;
        }

        SSA_PARENT_DEFS_FOREACH(&parent_defs, it)
        {
                if (it.pos->value.num_parents != num_parents)
                        continue;

                ssa_builder b;
                ssa_init_builder(&b, context, ssa_get_block_instrs_begin(block));
                ssa_value* phi = ssa_build_phi(&b, ssa_get_value_type(it.pos->value.def));
                ssa_scope_update(&md->phis, it.pos->key, phi);
                ssa_scope_update(&md->defs, it.pos->key, phi);
        }

        ssa_parent_defs_drop(&parent_defs);
}

static void ssa_promote_allocas_in_block(
        ssa_context* context, ssa_block* block, struct ssa_scope* promotable_allocas)
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
                        struct ssa_scope_entry* e = ssa_scope_lookup(&md->defs, it.pos->key);
                        if (!e)
                                continue;

                        ssa_instr* instr = ssa_get_var_instr(it.pos->value);
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

        struct ssa_scope promotable_allocas;
        ssa_scope_init(&promotable_allocas);

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
                ssa_scope_drop(&md->defs);
                ssa_scope_drop(&md->phis);
                dealloc(md);
        }

        ssa_scope_drop(&promotable_allocas);
}
