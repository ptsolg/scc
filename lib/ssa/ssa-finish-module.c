#include "ssa-emitter.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-value.h"

static bool ssa_start_transaction(ssa_module_emitter* self, ssa_block* entry, ssa_block* body)
{
        ssa_builder b;
        ssa_init_builder(&b, self->context, ssa_get_block_instrs_end(entry));

        ssa_value* onabort = ssa_build_call_0(&b, self->tm_info.start);
        if (!onabort)
                return false;

        ssa_value* isnull = ssa_build_neq_zero(&b, onabort);
        if (!isnull)
                return false;

        ssa_block* ontrue = ssa_new_block(self->context, false);
        ssa_value* body_label = ssa_get_block_label(body);
        if (!ssa_build_cond_jmp(&b, isnull, ssa_get_block_label(ontrue), body_label))
                return false;

        ssa_init_builder(&b, self->context, ssa_get_block_instrs_end(ontrue));
        ssa_value* val = ssa_build_call_1(&b, self->tm_info.setjmp, onabort);
        if (!val || !(val = ssa_build_neq_zero(&b, val)))
                return false;

        if (!ssa_build_cond_jmp(&b, val, ssa_get_block_label(entry), body_label))
                return false;

        ssa_add_block_after(ontrue, entry);
        return true;
}

static bool ssa_finish_atomic_block_terminator(ssa_module_emitter* self, ssa_instr* instr)
{
        bool exits_transaction = false;
        if (ssa_get_terminator_instr_kind(instr) == STIK_RETURN)
                exits_transaction = true;
        else
        {
                SSA_FOREACH_TERMINATOR_SUCCESSOR(instr, it, end)
                {
                        ssa_value* label = ssa_get_value_use_value(it);
                        if (!ssa_block_is_atomic(ssa_get_label_block(label)))
                        {
                                exits_transaction = true;
                                break;
                        }
                }
        }
        if (!exits_transaction)
                return true;

        ssa_builder b;
        ssa_init_builder(&b, self->context, NULL);
        return ssa_build_call_n_ex(&b, instr, self->tm_info.commit, NULL, 0, false) != NULL;
}

static ssa_value* ssa_emit_atomic_block_alloca(
        ssa_module_emitter* self, ssa_builder* builder, ssa_instr* ip, tree_type* t)
{
        size_t size = tree_get_sizeof(builder->context->target, t);
        ssa_value* size_val = ssa_build_size_t_constant(builder, size);
        if (!size_val)
                return NULL;

        ssa_value* storage = ssa_build_call_n_ex(
                builder, ip, self->tm_info.alloca, &size_val, 1, true);
        if (!storage)
                return NULL;
        if (!(storage = ssa_build_cast_ex(builder, ssa_get_var_instr(storage),
                tree_new_pointer_type(self->context->tree, t), storage, true)))
        {
                return NULL;
        }

        return storage;
}

static ssa_value* ssa_get_addressof(
        ssa_module_emitter* self, ssa_builder* builder, ssa_instr* alloca_ip, ssa_value* val)
{
        ssa_value_kind vk = ssa_get_value_kind(val);
        if (vk == SVK_GLOBAL_VAR
                || (vk == SVK_LOCAL_VAR && ssa_get_instr_kind(ssa_get_var_instr(val)) == SIK_ALLOCA))
        {
                return val;
        }

        ssa_value* storage = ssa_emit_atomic_block_alloca(
                self, builder, alloca_ip, ssa_get_value_type(val));
        return storage && ssa_build_store(builder, val, storage) ? storage : NULL;
}

static bool ssa_finish_atomic_block_store_instr(
        ssa_module_emitter* self, ssa_instr* alloca_ip, ssa_instr* instr)
{
        ssa_builder b;
        ssa_init_builder(&b, self->context, instr);

        ssa_value* value = ssa_get_instr_operand_value(instr, 0);
        tree_type* value_type = ssa_get_value_type(value);
        size_t value_size = tree_get_sizeof(self->context->target, value_type);
        ssa_value* dest = ssa_get_instr_operand_value(instr, 1);

        if (!(dest = ssa_build_cast_to_pvoid(&b, dest)))
                return false;

        if (tree_type_is_scalar(value_type) && value_size == self->tm_info.word_size)
        {
                ssa_value* word = ssa_build_cast(&b, self->tm_info.word, value);
                if (!word || !ssa_build_call_2(&b, self->tm_info.write_word, dest, word))
                        return false;

                ssa_remove_instr(instr);
                return true;
        }

        if (!(value = ssa_get_addressof(self, &b, alloca_ip, value)))
                return false;
        if (!(value = ssa_build_cast_to_pvoid(&b, value)))
                return false;
        
        ssa_value* size_val = ssa_build_u32_constant(&b, (unsigned)value_size);
        if (!size_val || !ssa_build_call_3(&b, self->tm_info.write, value, dest, size_val))
                return false;
         
        ssa_remove_instr(instr);
        return true;
}

static bool ssa_finish_atomic_block_alloca_instr(ssa_module_emitter* self, ssa_instr* instr)
{
        ssa_builder b;
        ssa_init_builder(&b, self->context, instr);
        ssa_value* storage = ssa_emit_atomic_block_alloca(self, &b, instr, ssa_get_allocated_type(instr));
        if (!storage)
                return false;

        ssa_replace_value_with(ssa_get_instr_var(instr), storage);
        ssa_remove_instr(instr);
        return true;
}

static bool ssa_finish_atomic_block_load_instr(
        ssa_module_emitter* self, ssa_instr* alloca_ip, ssa_instr* instr)
{
        ssa_builder b;
        ssa_init_builder(&b, self->context, instr);
       
        ssa_value* source = ssa_get_instr_operand_value(instr, 0);
        tree_type* source_type = tree_get_pointer_target(ssa_get_value_type(source));
        size_t source_size = tree_get_sizeof(self->context->target, source_type);

        if (!(source = ssa_build_cast_to_pvoid(&b, source)))
                return false;

        if (tree_type_is_scalar(source_type) && source_size == self->tm_info.word_size)
        {
                ssa_value* val = ssa_build_call_1(&b, self->tm_info.read_word, source);
                if (!val || !(val = ssa_build_cast(&b, source_type, val)))
                        return false;

                ssa_replace_value_with(ssa_get_instr_var(instr), val);
                ssa_remove_instr(instr);
                return true;
        }

        ssa_value* storage = ssa_emit_atomic_block_alloca(self, &b, alloca_ip, source_type);
        if (!storage)
                return false;

        ssa_value* storage_pvoid = ssa_build_cast_to_pvoid(&b, storage);
        if (!storage_pvoid)
                return false;
        if (!(source = ssa_build_cast_to_pvoid(&b, source)))
                return false;

        ssa_value* size_val = ssa_build_u32_constant(&b, (unsigned)source_size);
        if (!size_val || !ssa_build_call_3(&b, self->tm_info.read, source, storage_pvoid, size_val))
                return false;

        ssa_value* val = ssa_build_load(&b, storage);
        if (!val)
                return false;

        ssa_replace_value_with(ssa_get_instr_var(instr), val);
        ssa_remove_instr(instr);
        return true;
}

static bool ssa_finish_atomic_block_instr(
        ssa_module_emitter* self, ssa_instr* alloca_ip, ssa_instr* instr)
{
        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_ALLOCA)
                return ssa_finish_atomic_block_alloca_instr(self, instr);
        else if (k == SIK_LOAD)
                return ssa_finish_atomic_block_load_instr(self, alloca_ip, instr);
        else if (k == SIK_STORE)
                return ssa_finish_atomic_block_store_instr(self, alloca_ip, instr);
        else if (k == SIK_TERMINATOR)
                return ssa_finish_atomic_block_terminator(self, instr);

        return true;
}

static bool ssa_finish_atomic_block(
        ssa_module_emitter* self, bool first, ssa_instr** alloca_ip, ssa_block* block)
{
        if (first)
        {
                ssa_block* body = ssa_new_block(self->context, true);
                ssa_move_block_instrs(block, body);
                SSA_FOREACH_VALUE_USE_SAFE(ssa_get_block_label(block), it, next, end)
                        if (ssa_get_instr_kind(ssa_get_value_use_instr(it)) == SIK_PHI)
                                ssa_set_value_use_value(it, ssa_get_block_label(body));

                ssa_add_block_after(body, block);
                if (!ssa_start_transaction(self, block, body))
                        return false;

                block = body;
                *alloca_ip = ssa_get_block_instrs_end(body);
        }
        SSA_FOREACH_BLOCK_INSTR(block, it)
                if (!ssa_finish_atomic_block_instr(self, *alloca_ip, it))
                        return false;
        return true;
}

static bool ssa_finish_function(ssa_module_emitter* self, ssa_value* func)
{
        ssa_block* it = ssa_get_function_blocks_begin(func);
        ssa_block* end = ssa_get_function_blocks_end(func);
        while (it != end)
        {
                if (!ssa_block_is_atomic(it))
                {
                        it = ssa_get_next_block(it);
                        continue;
                }

                bool first = true;
                ssa_instr* alloca_ip;
                while (it != end && ssa_block_is_atomic(it))
                {
                        ssa_block* next = ssa_get_next_block(it);
                        if (!ssa_finish_atomic_block(self, first, &alloca_ip, it))
                                return false;
                        it = next;
                        first = false;
                }
        }
        return true;
}

extern ssa_module* ssa_finish_module(ssa_module_emitter* self)
{
        SSA_FOREACH_MODULE_GLOBAL(self->module, it, end)
                if (ssa_get_value_kind(*it) == SVK_FUNCTION)
                        if (!ssa_finish_function(self, *it))
                                return NULL;
        ssa_number_module_values(self->module);
        return self->module;
}