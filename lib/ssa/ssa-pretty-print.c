#include "scc/ssa/ssa-pretty-print.h"
#include "ssa-printer.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include <ctype.h>

static inline void ssa_print_id(ssa_printer* self, ssa_id id)
{
        ssa_printf(self, "%u", id);
}

static inline void ssa_print_label_ref(ssa_printer* self, ssa_id id)
{
        ssa_printc(self, '@');
        ssa_print_id(self, id);
}

static inline void ssa_print_string_value(ssa_printer* self, const ssa_value* val)
{
        strentry entry;
        if (!tree_get_id_strentry(ssa_get_tree(self->context), ssa_get_string_value(val), &entry))
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
        if (k == SVK_LOCAL_VAR || k == SVK_PARAM)
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
        else if (k == SVK_FUNCTION || k == SVK_GLOBAL_VAR)
        {
                ssa_printc(self, '%');
                tree_decl* entity = k == SVK_FUNCTION
                        ? ssa_get_function_entity(val)
                        : ssa_get_global_var_entity(val);
                ssa_prints(self, tree_get_id_string(ssa_get_tree(self->context),
                        tree_get_decl_name(entity)));
        }
        else if (k == SVK_STRING)
                ssa_print_string_value(self, val);
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
                case SMK_ACQ_REL:
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

static void ssa_print_instr(ssa_printer* self, const ssa_instr* instr)
{
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

static void ssa_print_block(ssa_printer* self, const ssa_block* block, bool print_preds)
{
        const ssa_value* label = ssa_get_block_clabel(block);
        ssa_print_value_ref(self, label);
        ssa_printc(self, ':');


        if (print_preds) // print predecessors
        {
                ssa_prints(self, " ; preds ");
                const ssa_value_use* use = ssa_get_value_uses_begin(label);
                const ssa_value_use* end = ssa_get_value_uses_cend(label);
                for (; use != end; use = ssa_get_next_value_use(use))
                {
                        ssa_instr* instr = ssa_get_value_use_instr(use);
                        if (ssa_get_instr_kind(instr) != SIK_TERMINATOR)
                                continue;
                        ssa_print_value_ref(self, ssa_get_block_label(ssa_get_instr_block(instr)));
                        ssa_printc(self, ' ');
                }
        }

        ssa_printc(self, '\n');

        self->indent_lvl++;
        SSA_FOREACH_BLOCK_INSTR(block, it)
        {
                ssa_print_indent(self);
                ssa_print_instr(self, it);
                ssa_printc(self, '\n');
        }
        self->indent_lvl--;
}

static bool ssa_print_function(ssa_printer* self, const ssa_value* func)
{
        tree_decl* entity = ssa_get_function_entity(func);
        if (!tree_get_func_body(entity))
                return false;

        ssa_printf(self, "; Definition for ");
        ssa_print_tree_id(self, tree_get_decl_name(entity));
        ssa_printc(self, '\n');

        SSA_FOREACH_FUNCTION_BLOCK(func, block)
        {
                ssa_print_block(self, block, false);
                ssa_printc(self, '\n');
        }
        return true;
}

extern void ssa_pretty_print_module(
        write_cb* cb, ssa_context* context, const ssa_module* module)
{
        ssa_printer p;
        ssa_init_printer(&p, context, cb);
        SSA_FOREACH_MODULE_GLOBAL(module, it, end)
                if (ssa_get_value_kind(*it) == SVK_FUNCTION && ssa_print_function(&p, *it))
                        ssa_printc(&p, '\n');
        ssa_dispose_printer(&p);
}