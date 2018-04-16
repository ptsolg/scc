#include "scc/ssa/ssa-printer.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-function.h"
#include "scc/ssa/ssa-module.h"
#include "scc/tree/tree-context.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h> // isprint

extern void ssa_init_printer(ssa_printer* self, write_cb* write, const ssa_context* context)
{
        writebuf_init(&self->buf, write);
        self->context = context;
}

extern void ssa_dispose_printer(ssa_printer* self)
{
        writebuf_flush(&self->buf);
        writebuf_dispose(&self->buf);
}

static inline void ssa_prints(ssa_printer* self, const char* s)
{
        if (s)
                writebuf_writes(&self->buf, s);
}

static inline void ssa_printc(ssa_printer* self, int c)
{
        writebuf_writec(&self->buf, c);
}

static inline void ssa_printf(ssa_printer* self, const char* f, ...)
{
        char buf[1024];

        va_list args;
        va_start(args, f);
        vsnprintf(buf, 1024, f, args);

        ssa_prints(self, buf);
}

static inline void ssa_print_indent(ssa_printer* self)
{
        ssa_prints(self, "    ");
}

static inline void ssa_print_endl(ssa_printer* self)
{
        ssa_printc(self, '\n');
}

static inline void ssa_print_id(ssa_printer* self, ssa_id id)
{
        ssa_printf(self, "%u", id);
}

static inline void ssa_print_label_ref(ssa_printer* self, ssa_id id)
{
        ssa_printc(self, '@');
        ssa_print_id(self, id);
}

static void ssa_print_string(ssa_printer* self, tree_id id)
{
        strentry entry;
        if (!tree_get_id_strentry(ssa_get_tree(self->context), id, &entry))
                return;

        ssa_printc(self, '"');
        for (size_t i = 0; i < entry.size; i++)
        {
                int c = entry.data[i];
                ssa_printf(self, (isprint(c) ? "\\%02X" : "%c"), c);
                
        }
        ssa_printc(self, '"');
}

static inline void ssa_print_value_ref(ssa_printer* self, const ssa_value* val)
{
        if (!val)
                return;

        ssa_value_kind k = ssa_get_value_kind(val);
        if (k == SVK_VARIABLE || k == SVK_PARAM)
        {
                ssa_printc(self, '$');
                ssa_print_id(self, ssa_get_value_id(val));
        }
        else if (k == SVK_LABEL)
        {
                ssa_printc(self, '@');
                ssa_print_id(self, ssa_get_value_id(val));
        }
        else if (k == SVK_CONSTANT)
        {
                char buf[64];
                avalue_print(ssa_get_constant_cvalue(val), buf, 64, 4);
                ssa_prints(self, buf);
        }
        else if (k == SVK_DECL)
        {
                ssa_printc(self, '%');
                tree_decl* entity = ssa_get_decl_entity(val);
                ssa_prints(self, tree_get_id_string(ssa_get_tree(self->context),
                        tree_get_decl_name(entity)));
        }
        else if (k == SVK_STRING)
                ssa_print_string(self, ssa_get_string_value(val));
}

static const char* ssa_binary_instr_table[] =
{
        "",
        "mul",
        "div",
        "mod",
        "add",
        "ptradd",
        "sub",
        "shl",
        "shr",
        "and",
        "or",
        "xor",
        "cmp le",
        "cmp gr",
        "cmp leq",
        "cmp geq",
        "cmp eq",
        "cmp neq",
};

static_assert(ARRAY_SIZE(ssa_binary_instr_table) == SBIK_SIZE,
        "ssa_binary_instr_table needs an update");

static const char* ssa_atomic_rmw_instr_table[] =
{
        "",
        "atomicrmw add",
        "atomicrmw xchg",
};

static_assert(ARRAY_SIZE(ssa_atomic_rmw_instr_table) == SARIK_SIZE,
        "ssa_binary_instr_table needs an update");

static const char* ssa_terminator_instr_table[] =
{
        "",
        "br",
        "br",
        "switch",
        "ret"
};

static_assert(ARRAY_SIZE(ssa_terminator_instr_table) == STIK_SIZE,
        "ssa_terminator_instr_table needs an update");

static const char* ssa_instr_table[] =
{
        "",
        "alloca",
        "load",
        "cast",
        "", // binop
        "store",
        "getfieldaddr",
        "call",
        "phi",
        "", // terminator
        "",  // atomic rmw
        "fence",
        "cmpxchg",
};

static_assert(ARRAY_SIZE(ssa_instr_table) == SIK_SIZE,
        "ssa_instr_table needs an update");

static void ssa_print_alloca_operand(ssa_printer* self, const ssa_instr* instr)
{
        const ssa_value* v = ssa_get_instr_cvar(instr);
        const tree_type* vt = ssa_get_value_type(v);
        const tree_target_info* ti = ssa_get_target(self->context);
        ssa_printf(self, "%u", (uint)tree_get_sizeof(ti, tree_get_pointer_target(vt)));
}

static void ssa_print_call_operands(ssa_printer* self, const ssa_instr* instr)
{
        ssa_value_use* it = ssa_get_instr_operands_begin(instr);
        ssa_value_use* end = ssa_get_instr_operands_end(instr);

        ssa_print_value_ref(self, ssa_get_value_use_value(it));
        ssa_prints(self, " (");
        for (it++; it != end; it++)
        {
                ssa_print_value_ref(self, ssa_get_value_use_value(it));
                if (it + 1 != end)
                        ssa_prints(self, ", ");
        }
        ssa_prints(self, ")");
}

static void ssa_print_phi_operands(ssa_printer* self, const ssa_instr* instr)
{
        SSA_FOREACH_INSTR_OPERAND(instr, it, end)
        {
                ssa_print_value_ref(self, ssa_get_value_use_value(it));
                it++; // skip block

                if (it + 1 != end)
                        ssa_prints(self, ", ");
        }
}

static void ssa_print_getfieldaddr_operands(ssa_printer* self, const ssa_instr* instr)
{
        SSA_FOREACH_INSTR_OPERAND(instr, it, end)
        {
                ssa_print_value_ref(self, ssa_get_value_use_value(it));
                ssa_prints(self, ", ");
        }

        ssa_printf(self, "%u", ssa_get_getfieldaddr_index(instr));
}

static void ssa_print_switch_instr_operands(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value_ref(self, ssa_get_instr_operand_value(instr, 0));
        ssa_prints(self, ", ");
        ssa_print_value_ref(self, ssa_get_instr_operand_value(instr, 1));

        ssa_prints(self, " [");
        size_t num_ops = ssa_get_instr_operands_size(instr);
        for (size_t i = 2; i < num_ops; i++)
        {
                ssa_print_value_ref(self, ssa_get_instr_operand_value(instr, i));
                if (i + 1 != num_ops)
                        ssa_prints(self, ", ");
        }
        ssa_prints(self, "]");
}

static void ssa_print_instr_operands(ssa_printer* self, const ssa_instr* instr)
{
        SSA_FOREACH_INSTR_OPERAND(instr, it, end)
        {
                ssa_print_value_ref(self, ssa_get_value_use_value(it));
                if (it + 1 != end)
                        ssa_prints(self, ", ");
        }
}

static void ssa_print_memorder(ssa_printer* self, ssa_memorder_kind kind)
{
        switch (kind)
        {
                case SMK_MONOTONIC:
                        ssa_prints(self, "monotonic");
                        return;
                case SMK_ACQUIRE:
                        ssa_prints(self, "acq");
                        return;
                case SMK_RELEASE:
                        ssa_prints(self, "rel");
                        return;
                case SMK_ACQUIRE_RELEASE:
                        ssa_prints(self, "acq_rel");
                        return;
                case SMK_SEQ_CST:
                        ssa_prints(self, "seq_cst");
                        return;
                default:
                        return;
        }
}

static void ssa_print_syncscope(ssa_printer* self, ssa_syncscope_kind kind)
{
        switch (kind)
        {
                case SSK_SINGLE_THREAD:
                        ssa_prints(self, "single_thread");
                        return;
                default:
                        return;
        }
}

extern void ssa_print_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_endl(self);
        ssa_print_indent(self);

        ssa_instr_kind k = ssa_get_instr_kind(instr);
        SSA_ASSERT_INSTR_KIND(k);

        if (ssa_instr_has_var(instr))
        {
                ssa_print_value_ref(self, ssa_get_instr_cvar(instr));
                ssa_prints(self, " = ");
        }
        
        const char* instr_name = ssa_instr_table[k];
        if (k == SIK_BINARY)
                instr_name = ssa_binary_instr_table[ssa_get_binop_kind(instr)];
        else if (k == SIK_TERMINATOR)
                instr_name = ssa_terminator_instr_table[ssa_get_terminator_instr_kind(instr)];
        else if (k == SIK_ATOMIC_RMW)
                instr_name = ssa_atomic_rmw_instr_table[ssa_get_atomic_rmw_instr_kind(instr)];

        ssa_printf(self, "%s ", instr_name);

        if (k == SIK_ALLOCA)
                ssa_print_alloca_operand(self, instr);
        else if (k == SIK_CALL)
                ssa_print_call_operands(self, instr);
        else if (k == SIK_PHI)
                ssa_print_phi_operands(self, instr);
        else if (k == SIK_GETFIELDADDR)
                ssa_print_getfieldaddr_operands(self, instr);
        else if (k == SIK_TERMINATOR && ssa_get_terminator_instr_kind(instr) == STIK_SWITCH)
                ssa_print_switch_instr_operands(self, instr);
        else
                ssa_print_instr_operands(self, instr);
       
        if (ssa_get_instr_operands_size(instr))
                ssa_printc(self, ' ');

        if (k == SIK_ATOMIC_RMW)
                ssa_print_memorder(self, ssa_get_atomic_rmw_instr_ordering(instr));
        else if (k == SIK_FENCE)
        {
                ssa_print_syncscope(self, ssa_get_fence_instr_syncscope(instr));
                ssa_printc(self, ' ');
                ssa_print_memorder(self, ssa_get_fence_instr_ordering(instr));
        }
        else if (k == SIK_ATOMIC_CMPXCHG)
        {
                ssa_print_memorder(self, ssa_get_atomic_cmpxchg_instr_success_ordering(instr));
                ssa_printc(self, ' ');
                ssa_print_memorder(self, ssa_get_atomic_cmpxchg_instr_failure_ordering(instr));
        }
}

extern void ssa_print_block(ssa_printer* self, const ssa_block* block)
{
        ssa_print_endl(self);
        ssa_print_value_ref(self, ssa_get_block_clabel(block));
        ssa_printc(self, ':');
        SSA_FOREACH_BLOCK_INSTR(block, it)
                ssa_print_instr(self, it);
        ssa_print_endl(self);
}

extern void ssa_print_function(ssa_printer* self, const ssa_function* func)
{
        tree_decl* entity = ssa_get_function_entity(func);
        const char* name = tree_get_id_string(
                ssa_get_tree(self->context), tree_get_decl_name(entity));
        ssa_printf(self, "; Definition for %s", name);

        SSA_FOREACH_FUNCTION_BLOCK(func, block)
                ssa_print_block(self, block);
        ssa_print_endl(self);
}

extern void ssa_print_module(ssa_printer* self, const ssa_module* module)
{
        SSA_FOREACH_MODULE_DEF(module, def)
                ssa_print_function(self, def);
}