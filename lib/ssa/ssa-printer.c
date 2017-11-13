#include "scc/ssa/ssa-printer.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-module.h"
#include "scc/tree/tree-context.h"
#include <stdio.h>
#include <stdarg.h>

extern void ssa_init_printer(ssa_printer* self,
        write_cb* write, const tree_context* context)
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

static inline void ssa_print_value_ref(ssa_printer* self, const ssa_value* val)
{
        if (!val)
        {
                ssa_prints(self, "void");
                return;
        }

        ssa_value_kind k = ssa_get_value_kind(val);
        if (k == SVK_VARIABLE)
        {
                ssa_printc(self, '$');
                ssa_print_id(self, ssa_get_var_id(val));
        }
        else if (k == SVK_CONSTANT)
                ssa_printc(self, '1');
}

static const char* ssa_binary_instr_table[SBIK_SIZE] =
{
        "",
        "mul",
        "div",
        "mod",
        "add",
        "sub",
        "shl",
        "shr",
        "and",
        "or",
        "xor",
        "le",
        "gr",
        "leq",
        "geq",
        "eq",
        "neq",
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssa_binary_instr_table) == SBIK_SIZE,
        "ssa_binary_instr_table needs an update");

static void ssa_print_binary_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_binary_instr_kind k = ssa_get_binop_opcode(instr);
        SSA_CHECK_BINARY_INSTR_KIND(k);
        ssa_printf(self, "%s ", ssa_binary_instr_table[k]);
        ssa_print_value_ref(self, ssa_get_binop_lhs(instr));
        ssa_prints(self, ", ");
        ssa_print_value_ref(self, ssa_get_binop_rhs(instr));
}

static void ssa_print_init_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value_ref(self, ssa_get_init_value(instr));
}

extern void ssa_print_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_instr_kind k = ssa_get_instr_kind(instr);
        SSA_CHECK_INSTR_KIND(k);

        switch (k)
        {
                case SIK_BINARY:
                        ssa_print_binary_instr(self, instr);
                        break;

                case SIK_INIT:
                        ssa_print_init_instr(self, instr);
                        break;

                case SIK_CAST:
                case SIK_CALL:
                case SIK_GETADDR:
                case SIK_GETPTRVAL:
                case SIK_PHI:
                        //todo
                default:
                        break;
        }
}

extern void ssa_print_value(ssa_printer* self, const ssa_value* value)
{
        ssa_print_endl(self);
        ssa_print_indent(self);
        ssa_print_value_ref(self, value);
        ssa_prints(self, " = ");
        ssa_print_instr(self, ssa_get_var_init(value));
}

extern void ssa_print_block(ssa_printer* self, const ssa_block* block)
{
        ssa_print_endl(self);
        ssa_print_label_ref(self, ssa_get_block_entry_id(block));
        ssa_printc(self, ':');

        SSA_FOREACH_BLOCK_VALUE(block, it)
                ssa_print_value(self, *it);
        ssa_print_endl(self);
}

extern void ssa_print_module(ssa_printer* self, const ssa_module* module)
{
        for (hiter def = ssa_get_module_begin(module);
                hiter_valid(&def); hiter_advance(&def))
        {
                tree_id id = hiter_get_key(&def);
                const char* name = tree_context_get_id(self->context, id);

                ssa_printf(self, "; Definition for %s", name);

                ssa_print_block(self, hiter_get_ptr(&def));
        }
}