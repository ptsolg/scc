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

static inline void ssa_print_value_ref(ssa_printer* self, const ssa_value* val)
{
        if (!val)
                return;

        ssa_value_kind k = ssa_get_value_kind(val);
        if (k == SVK_VARIABLE)
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
                avalue v = ssa_get_constant_value(val);
                avalue_print(&v, buf, 64, 4);
                ssa_prints(self, buf);
        }
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
        "cmp le",
        "cmp gr",
        "cmp leq",
        "cmp geq",
        "cmp eq",
        "cmp neq",
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssa_binary_instr_table) == SBIK_SIZE,
        "ssa_binary_instr_table needs an update");

static void ssa_print_binary_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value_ref(self, ssa_get_instr_cvalue(instr));
        ssa_binary_instr_kind k = ssa_get_binop_opcode(instr);
        SSA_CHECK_BINARY_INSTR_KIND(k);
        ssa_printf(self, " = %s ", ssa_binary_instr_table[k]);
        ssa_print_value_ref(self, ssa_get_binop_lhs(instr));
        ssa_prints(self, ", ");
        ssa_print_value_ref(self, ssa_get_binop_rhs(instr));
}

static void ssa_print_alloca(ssa_printer* self, const ssa_instr* instr)
{
        const ssa_value* v = ssa_get_instr_cvalue(instr);
        const tree_target_info* ti = ssa_get_context_target(self->context);
        const tree_type* vt = ssa_get_value_type(v);
        ssa_print_value_ref(self, v);
        ssa_printf(self, " = alloca %u",
                (uint)tree_get_sizeof(ti, tree_get_pointer_target(vt)));
}

static void ssa_print_load(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value_ref(self, ssa_get_instr_cvalue(instr));
        ssa_prints(self, " = load ");
        ssa_print_value_ref(self, ssa_get_load_what(instr));
}

static void ssa_print_store(ssa_printer* self, const ssa_instr* instr)
{
        ssa_prints(self, "store ");
        ssa_print_value_ref(self, ssa_get_store_what(instr));
        ssa_prints(self, ", ");
        ssa_print_value_ref(self, ssa_get_store_where(instr));
}

static void ssa_print_phi(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value_ref(self, ssa_get_instr_cvalue(instr));
        ssa_prints(self, " = phi ");
        SSA_FOREACH_PHI_VAR(instr, it)
        {
                ssa_print_value_ref(self, *it);
                if (it + 1 != ssa_get_phi_end(instr))
                        ssa_prints(self, ", ");
        }
}

static void ssa_print_cast(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value_ref(self, ssa_get_instr_cvalue(instr));
        ssa_prints(self, " = cast ");
        ssa_print_value_ref(self, ssa_get_cast_operand(instr));
}

extern void ssa_print_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_endl(self);
        ssa_print_indent(self);

        ssa_instr_kind k = ssa_get_instr_kind(instr);
        SSA_CHECK_INSTR_KIND(k);

        if (k == SIK_ALLOCA)
                ssa_print_alloca(self, instr);
        else if (k == SIK_LOAD)
                ssa_print_load(self, instr);
        else if (k == SIK_STORE)
                ssa_print_store(self, instr);
        else if (k == SIK_BINARY)
                ssa_print_binary_instr(self, instr);
        else if (k == SIK_PHI)
                ssa_print_phi(self, instr);
        else if (k == SIK_CAST)
                ssa_print_cast(self, instr);
}

static void ssa_print_branch(ssa_printer* self, const ssa_branch* br)
{
        if (!br)
                return;

        ssa_print_endl(self);
        ssa_print_indent(self);

        ssa_branch_kind k = ssa_get_branch_kind(br);
        if (k == SBK_JUMP)
        {
                ssa_prints(self, "br ");
                ssa_print_value_ref(self, ssa_get_jump_dest(br));
        }
        else if (k == SBK_IF)
        {
                ssa_prints(self, "br ");
                ssa_print_value_ref(self, ssa_get_if_cond(br));
                ssa_prints(self, ", ");
                ssa_print_value_ref(self, ssa_get_if_true_block(br));
                ssa_prints(self, ", ");
                ssa_print_value_ref(self, ssa_get_if_false_block(br));
        }
        else if (k == SBK_RETURN)
        {
                ssa_prints(self, "ret ");
                ssa_print_value_ref(self, ssa_get_return_value(br));
        }
}

extern void ssa_print_block(ssa_printer* self, const ssa_block* block)
{
        ssa_print_endl(self);
        ssa_print_value_ref(self, ssa_get_block_cvalue(block));
        ssa_printc(self, ':');

        SSA_FOREACH_BLOCK_INSTR(block, it)
                ssa_print_instr(self, it);
        ssa_print_branch(self, ssa_get_block_exit(block));

        ssa_print_endl(self);
}

extern void ssa_print_function(ssa_printer* self, const ssa_function* func)
{
        tree_decl* entity = ssa_get_function_entity(func);
        const char* name = tree_context_get_id(
                ssa_get_context_tree(self->context), tree_get_decl_name(entity));
        ssa_printf(self, "; Definition for %s", name);

        SSA_FOREACH_FUNCTION_BLOCK(func, block)
                ssa_print_block(self, block);
        ssa_print_endl(self);
}

extern void ssa_print_module(ssa_printer* self, const ssa_module* module)
{
        for (hiter func = ssa_get_module_begin(module);
                hiter_valid(&func); hiter_advance(&func))
        {
                ssa_print_function(self, hiter_get_ptr(&func));
        }
}