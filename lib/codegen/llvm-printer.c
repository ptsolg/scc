#include "scc/codegen/llvm-printer.h"
#include "scc/ssa/ssa.h"
#include "scc/tree/tree-type.h"
#include "scc/tree/tree-module.h"
#include "scc/scl/char-info.h"
#include <stdarg.h>
#include <stdio.h>

extern void llvm_printer_init(llvm_printer* self, write_cb* write, const ssa_context* ssa)
{
        self->ssa = ssa;
        self->tree = ssa_get_tree(ssa);
        writebuf_init(&self->buf, write);
}

extern void llvm_printer_dispose(llvm_printer* self)
{
        writebuf_flush(&self->buf);
        writebuf_dispose(&self->buf);
}

static void llvm_prints(llvm_printer* self, const char* s)
{
        if (s)
                writebuf_writes(&self->buf, s);
}

static void llvm_printf(llvm_printer* self, const char* f, ...)
{
        char buf[1024];

        va_list args;
        va_start(args, f);
        vsnprintf(buf, 1024, f, args);

        llvm_prints(self, buf);
}

static void llvm_printc(llvm_printer* self, int c)
{
        writebuf_writec(&self->buf, c);
}

static void llvm_print_wspace(llvm_printer* self)
{
        llvm_printc(self, ' ');
}

static void llvm_print_endl(llvm_printer* self)
{
        llvm_printc(self, '\n');
}

static void llvm_print_indent(llvm_printer* self)
{
        llvm_prints(self, "    ");
}

static const char* llvm_builtin_type_table[] = 
{
        "invalid",
        "void",
        "i8",
        "i8",
        "i16",
        "i16",
        "i32",
        "i32",
        "i64",
        "i64",
        "float",
        "double",
};

S_STATIC_ASSERT(S_ARRAY_SIZE(llvm_builtin_type_table) == TBTK_SIZE,
        "llvm_builtin_type_table needs an update");

static void llvm_printer_emit_builtin_type(llvm_printer* self, const tree_type* type)
{
        llvm_prints(self, llvm_builtin_type_table[tree_get_builtin_type_kind(type)]);
}

static void llvm_printer_emit_pointer_type(llvm_printer* self, const tree_type* type)
{
        llvm_printer_emit_type(self, tree_get_pointer_target(type));
        llvm_printc(self, '*');
}

static void llvm_printer_emit_function_type_params(llvm_printer* self, const tree_type* type)
{
        llvm_printc(self, '(');
        TREE_FOREACH_FUNCTION_TYPE_PARAM(type, it)
        {
                llvm_printer_emit_type(self, *it);
                if (it + 1 != tree_get_function_type_params_end(type))
                        llvm_prints(self, ", ");
        }
        if (tree_function_type_is_vararg(type))
                llvm_prints(self, ", ...");
        llvm_printc(self, ')');
}

static void llvm_printer_emit_function_type(llvm_printer* self, const tree_type* type)
{
        llvm_printer_emit_type(self, tree_get_function_type_result(type));
        llvm_print_wspace(self);
        llvm_printer_emit_function_type_params(self, type);
}

static void llvm_printer_emit_array_type(llvm_printer* self, const tree_type* type)
{
        if (tree_get_array_kind(type) != TAK_CONSTANT)
                return;

        llvm_printf(self, "[%u x ", int_get_u32(tree_get_constant_array_size_cvalue(type)));
        llvm_printer_emit_type(self, tree_get_array_eltype(type));
        llvm_printc(self, ']');
}

extern void llvm_printer_emit_type(llvm_printer* self, const tree_type* type)
{
        type = tree_desugar_ctype(type);
        tree_type_kind k = tree_get_type_kind(type);

        if (k == TTK_BUILTIN)
                llvm_printer_emit_builtin_type(self, type);
        else if (k == TTK_FUNCTION)
                llvm_printer_emit_function_type(self, type);
        else if (k == TTK_POINTER)
                llvm_printer_emit_pointer_type(self, type);
        else if (k == TTK_ARRAY)
                llvm_printer_emit_array_type(self, type);
}

extern void llvm_printer_emit_value(
        llvm_printer* self, const ssa_value* value, bool emit_type)
{
        ssa_value_kind k = ssa_get_value_kind(value);
        if (k == SVK_LABEL)
        {
                llvm_printf(self, "%%%u", ssa_get_value_id(value));
                return;
        }

        if (emit_type)
        {
                llvm_printer_emit_type(self, ssa_get_value_type(value));
                if (k == SVK_STRING)
                        llvm_printc(self, '*');
                llvm_print_wspace(self);
        }

        if (k == SVK_VARIABLE)
                llvm_printf(self, "%%%u", ssa_get_value_id(value));
        else if (k == SVK_CONSTANT)
        {
                char num[64];
                avalue val = ssa_get_constant_value(value);
                avalue_print(&val, num, S_ARRAY_SIZE(num), 4);
                llvm_prints(self, num);
        }
        else if (k == SVK_DECL)
                llvm_printf(self, "@%s", tree_get_id_cstr(self->tree,
                        tree_get_decl_name(ssa_get_decl_entity(value))));
        else if (k == SVK_STRING)
                llvm_printf(self, "@.str.%u", ssa_get_value_id(value));
}

static void llvm_printer_emit_alloca(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, var, false);
        llvm_prints(self, " = alloca ");
        llvm_printer_emit_type(self, ssa_get_allocated_type(instr));
}

typedef enum
{
        LBIK_SIGNED,
        LBIK_UNSIGNED,
        LBIK_FLOATING,
        LBIK_SIZE
} llvm_binary_instr_kind;

static llvm_binary_instr_kind llvm_get_binary_instr_kind(const ssa_instr* instr)
{
        const tree_type* t = ssa_get_value_type(ssa_get_instr_cvar(instr));
        if (tree_type_is_floating(t))
                return LBIK_FLOATING;

        return tree_type_is_signed_integer(t) ? LBIK_SIGNED : LBIK_UNSIGNED;
}

static const char* llvm_binary_instr_table[SBIK_SIZE][LBIK_SIZE] =
{
        { NULL,       NULL,       NULL,  },
        { "mul nsw",  "mul",      "fmul", },
        { "sdiv",     "udiv",     "fdiv", },
        { "srem",     "urem",     "frem", },
        { "add nsw",  "add",      "fadd", },
        { "sub nsw",  "sub",      "fsub", },
        { "shl",      "shl",      NULL, },
        { "shr",      "shr",      NULL, },
        { "and",      "and",      NULL, },
        { "or",       "or",       NULL, },
        { "xor",      "xor",      NULL, },
        { "icmp slt", "icmp ult", "fcmp olt", },
        { "icmp sgt", "icmp ugt", "fcmp ogt", },
        { "icmp sle", "icmp ule", "fcmp ole", },
        { "icmp sge", "icmp uge", "fcmp oge", },
        { "icmp eq",  "icmp eq",  "fcmp oeq", },
        { "icmp ne",  "icmp ne",  "fcmp one", },
};

static void llvm_printer_emit_binary_instr(llvm_printer* self, const ssa_instr* instr)
{
        ssa_binary_instr_kind k = ssa_get_binop_kind(instr);
        llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
        llvm_prints(self, " = ");

        llvm_binary_instr_kind lk = llvm_get_binary_instr_kind(instr);
        llvm_printf(self, "%s ", llvm_binary_instr_table[k][lk]);
        llvm_printer_emit_value(self, ssa_get_binop_lhs(instr), true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_binop_rhs(instr), false);
}

static void llvm_printer_emit_store(llvm_printer* self, const ssa_instr* instr)
{
        llvm_prints(self, "store ");
        llvm_printer_emit_value(self, ssa_get_store_what(instr), true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_store_where(instr), true);
}

static void llvm_printer_emit_load(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* result = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, result, false);
        llvm_prints(self, " = load ");
        llvm_printer_emit_type(self, ssa_get_value_type(result));
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_load_what(instr), true);
}

static void llvm_printer_emit_function_header(llvm_printer*, const char*, const tree_decl*);

static void llvm_printer_emit_call(llvm_printer* self, const ssa_instr* instr)
{
        if (ssa_instr_has_var(instr))
        {
                llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
                llvm_prints(self, " = ");
        }
        llvm_prints(self, "call ");

        ssa_value* func = ssa_get_called_func(instr);
        llvm_printer_emit_type(self, tree_get_pointer_target(ssa_get_value_type(func)));
        llvm_print_wspace(self);
        llvm_printer_emit_value(self, ssa_get_called_func(instr), false);

        llvm_printc(self, '(');
        SSA_FOREACH_CALL_ARG(instr, it, end)
        {
                llvm_printer_emit_value(self, *it, true);
                if (it + 1 != end)
                        llvm_prints(self, ", ");
        }
        llvm_printc(self, ')');
}

typedef enum
{
        LSK_INVALID,
        LSK_TRUNC,
        LSK_ZEXT,
        LSK_SEXT,
        LSK_FPTRUNC,
        LSK_FPEXT,
        LSK_FPTOUI,
        LSK_FPTOSI,
        LSK_UITOFP,
        LSK_SITOFP,
        LSK_PTRTOINT,
        LSK_INTTOPTR,
        LSK_BITCAST,
        LSK_SIZE,
} llvm_cast_kind;

static llvm_cast_kind llvm_get_cast_kind(const tree_target_info* target,
        const tree_type* from, const tree_type* to)
{
        ssize from_size = tree_get_sizeof(target, from);
        ssize to_size = tree_get_sizeof(target, to);
        if (tree_type_is_pointer(to))
                return tree_type_is_integer(from) ? LSK_INTTOPTR : LSK_BITCAST;
        else if (tree_type_is_pointer(from))
                return tree_type_is_integer(to) ? LSK_PTRTOINT : LSK_BITCAST;

        bool from_float = tree_type_is_floating(from);
        bool to_float = tree_type_is_floating(to);
        if (from_float || to_float)
        {
                if (from_float && to_float)
                {
                        return from_size < to_size
                                ? LSK_FPEXT
                                : from_size == to_size ? LSK_BITCAST : LSK_FPTRUNC;
                }

                if (from_float)
                        return tree_type_is_signed_integer(to) ? LSK_FPTOSI : LSK_FPTOUI;
                return tree_type_is_signed_integer(from) ? LSK_SITOFP : LSK_UITOFP;

        }

        if (to_size <= from_size)
                return to_size == from_size ? LSK_BITCAST : LSK_TRUNC;

        return tree_type_is_signed_integer(from) ? LSK_SEXT : LSK_ZEXT;
}

static const char* llvm_cast_table[LSK_SIZE] = 
{
        "invalid",
        "trunc",
        "zext",
        "sext",
        "fptrunc",
        "fpext",
        "fptoui",
        "fptosi",
        "uitofp",
        "sitofp",
        "ptrtoint",
        "inttoptr",
        "bitcast",
};

static void llvm_printer_emit_cast(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        const ssa_value* operand = ssa_get_cast_operand(instr);
        llvm_printer_emit_value(self, var, false);
        llvm_prints(self, " = ");

        tree_type* to = ssa_get_value_type(var);
        llvm_cast_kind kind = llvm_get_cast_kind(
                ssa_get_target(self->ssa), ssa_get_value_type(operand), to);
        if (kind == LSK_INVALID)
                return;

        llvm_printf(self, "%s ", llvm_cast_table[kind]);
        llvm_printer_emit_value(self, operand, true);
        llvm_prints(self, " to ");
        llvm_printer_emit_type(self, to);
}

static void llvm_printer_emit_getaddr(llvm_printer* self, const ssa_instr* instr)
{
        ssa_value* operand = ssa_get_getaddr_operand(instr);
        llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
        llvm_prints(self, " = getelementptr inbounds ");
        llvm_printer_emit_type(self, tree_get_pointer_target(ssa_get_value_type(operand)));
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, operand, true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_getaddr_index(instr), true);
}

extern void llvm_printer_emit_instr(llvm_printer* self, const ssa_instr* instr)
{
        llvm_print_endl(self);
        llvm_print_indent(self);

        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_ALLOCA)
                llvm_printer_emit_alloca(self, instr);
        else if (k == SIK_BINARY)
                llvm_printer_emit_binary_instr(self, instr);
        else if (k == SIK_STORE)
                llvm_printer_emit_store(self, instr);
        else if (k == SIK_LOAD)
                llvm_printer_emit_load(self, instr);
        else if (k == SIK_CALL)
                llvm_printer_emit_call(self, instr);
        else if (k == SIK_CAST)
                llvm_printer_emit_cast(self, instr);
        else if (k == SIK_GETADDR)
                llvm_printer_emit_getaddr(self, instr);
}

static void llvm_printer_emit_branch(llvm_printer* self, const ssa_branch* br)
{
        llvm_print_endl(self);
        llvm_print_indent(self);

        ssa_branch_kind k = ssa_get_branch_kind(br);
        if (k == SBK_RETURN)
        {
                llvm_prints(self, "ret ");
                ssa_value* val = ssa_get_return_value(br);
                if (val)
                        llvm_printer_emit_value(self, val, true);
                else
                        llvm_prints(self, "void");
        }
        else if (k == SBK_IF)
        {
                llvm_prints(self, "br i1 ");
                llvm_printer_emit_value(self, ssa_get_if_cond(br), false);
                llvm_prints(self, ", label ");
                llvm_printer_emit_value(self, ssa_get_if_true_block(br), true);
                llvm_prints(self, ", label ");
                llvm_printer_emit_value(self, ssa_get_if_false_block(br), true);
        }
        else if (k == SBK_JUMP)
        {
                llvm_prints(self, "br label ");
                llvm_printer_emit_value(self, ssa_get_jump_dest(br), false);
        }
}

extern void llvm_printer_emit_block(llvm_printer* self, const ssa_block* block)
{
        llvm_print_endl(self);
        llvm_printf(self, "; <label>:%u", ssa_get_value_id(ssa_get_block_clabel(block)));

        SSA_FOREACH_BLOCK_INSTR(block, instr)
                llvm_printer_emit_instr(self, instr);

        llvm_printer_emit_branch(self, ssa_get_block_exit(block));
}

static void llvm_printer_emit_function_header(
        llvm_printer* self, const char* prefix, const tree_decl* func)
{
        llvm_prints(self, prefix);
        tree_type* t = tree_get_decl_type(func);
        llvm_printer_emit_type(self, tree_get_function_type_result(t));
        llvm_printf(self, " @%s", tree_get_id_cstr(self->tree, tree_get_decl_name(func)));
        llvm_printer_emit_function_type_params(self, t);
}

extern void llvm_printer_emit_decl(llvm_printer* self, const tree_decl* decl)
{
        llvm_print_endl(self);
        tree_decl_kind k = tree_get_decl_kind(decl);
        if (k == TDK_FUNCTION)
                llvm_printer_emit_function_header(self, "declare ", decl);
}

extern void llvm_printer_emit_function_decl(llvm_printer* self, const ssa_function* func)
{
        llvm_print_endl(self);
        llvm_printer_emit_function_header(self, "define ", ssa_get_function_entity(func));
        llvm_print_endl(self);
        llvm_printc(self, '{');
        SSA_FOREACH_FUNCTION_BLOCK(func, block)
                llvm_printer_emit_block(self, block);
        llvm_print_endl(self);
        llvm_printc(self, '}');
        llvm_print_endl(self);
}

static void llvm_printer_emit_global(llvm_printer* self, ssa_value* global)
{
        llvm_print_endl(self);
        if (ssa_get_value_kind(global) != SVK_STRING)
                S_UNREACHABLE();

        llvm_printer_emit_value(self, global, false);
        llvm_prints(self, " = private constant ");
        llvm_printer_emit_type(self, ssa_get_value_type(global));

        strentry entry;
        if (!tree_get_id_strentry(self->tree, ssa_get_string_value(global), &entry))
                return;

        llvm_prints(self, " c\"");
        for (ssize i = 0; i < entry.size; i++)
        {
                int c = entry.data[i];
                llvm_printf(self, (char_is_escape(c) ? "\\%02X" : "%c"), c);

        }
        llvm_printc(self, '"');
}

extern void llvm_printer_emit_module(
        llvm_printer* self, const ssa_module* sm, const tree_module* tm)
{
        SSA_FOREACH_MODULE_GLOBAL(sm, it, end)
                llvm_printer_emit_global(self, *it);

        const tree_decl_scope* globals = tree_get_module_cglobals(tm);
        TREE_FOREACH_DECL_IN_SCOPE(globals, decl)
        {
                ssa_function* func;
                if (tree_decl_is(decl, TDK_FUNCTION) && (func = ssa_module_lookup(sm, decl)))
                {
                        if (!tree_get_function_body(decl))
                                continue;

                        llvm_printer_emit_function_decl(self, func);
                }
                else
                        llvm_printer_emit_decl(self, decl);
        }
}