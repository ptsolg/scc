#include "scc/codegen/llvm-printer.h"
#include "scc/ssa/ssa.h"
#include "scc/tree/tree-type.h"
#include "scc/tree/tree-module.h"
#include "scc/core/dseq-instance.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h> // isprint

#define HTAB_TYPE recmap
#define HTAB_IMPL_FN_GENERATOR(NAME) _recmap_##NAME
#define HTAB_KEY_TYPE const void*
#define HTAB_DELETED_KEY ((const void*)0)
#define HTAB_EMPTY_KEY ((const void*)1)
#define HTAB_VALUE_TYPE ssa_id
#define HTAB_INIT recmap_init
#define HTAB_INIT_ALLOC recmap_init_alloc
#define HTAB_DISPOSE recmap_dispose
#define HTAB_GET_SIZE recmap_size
#define HTAB_GET_ALLOCATOR recmap_alloc
#define HTAB_RESERVE recmap_reserve
#define HTAB_CLEAR recmap_clear
#define HTAB_ERASE recmap_erase
#define HTAB_GROW recmap_grow
#define HTAB_INSERT recmap_insert
#define HTAB_FIND recmap_find

#define HTAB_ITERATOR_TYPE recmap_iter
#define HTAB_ITERATOR_GET_KEY recmap_iter_key
#define HTAB_ITERATOR_ADVANCE recmap_iter_advance
#define HTAB_ITERATOR_INIT recmap_iter_init
#define HTAB_ITERATOR_CREATE recmap_iter_create
#define HTAB_ITERATOR_IS_VALID recmap_iter_valid
#define HTAB_ITERATOR_GET_VALUE recmap_iter_value

#include "scc/core/htab.h"

#undef HTAB_TYPE
#undef HTAB_IMPL_FN_GENERATOR
#undef HTAB_KEY_TYPE 
#undef HTAB_DELETED_KEY
#undef HTAB_EMPTY_KEY
#undef HTAB_VALUE_TYPE
#undef HTAB_INIT
#undef HTAB_INIT_ALLOC
#undef HTAB_DISPOSE
#undef HTAB_GET_SIZE
#undef HTAB_GET_ALLOCATOR
#undef HTAB_RESERVE
#undef HTAB_CLEAR
#undef HTAB_ERASE
#undef HTAB_GROW
#undef HTAB_INSERT
#undef HTAB_FIND
#undef HTAB_ITERATOR_TYPE
#undef HTAB_ITERATOR_GET_KEY
#undef HTAB_ITERATOR_ADVANCE
#undef HTAB_ITERATOR_INIT
#undef HTAB_ITERATOR_CREATE
#undef HTAB_ITERATOR_IS_VALID
#undef HTAB_ITERATOR_GET_VALUE

extern void llvm_printer_init(llvm_printer* self, write_cb* write, ssa_context* ssa)
{
        self->ssa = ssa;
        self->tree = ssa_get_tree(ssa);
        self->tmp_id = 0;
        self->indent_lvl = 0;
        self->rec_id = 0;
        recmap_init_alloc(&self->rec_to_id, ssa_get_alloc(ssa));
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
        for (int i = 0; i < self->indent_lvl; i++)
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

static_assert(ARRAY_SIZE(llvm_builtin_type_table) == TBTK_SIZE,
        "llvm_builtin_type_table needs an update");

static void llvm_printer_emit_builtin_type(llvm_printer* self, const tree_type* type)
{
        llvm_prints(self, llvm_builtin_type_table[tree_get_builtin_type_kind(type)]);
}

static void llvm_printer_emit_pointer_type(llvm_printer* self, const tree_type* type)
{
        tree_type* target = tree_get_pointer_target(type);
        if (tree_type_is_void(target))
                llvm_prints(self, llvm_builtin_type_table[TBTK_INT8]);
        else
                llvm_printer_emit_type(self, tree_get_pointer_target(type));
        llvm_printc(self, '*');
}

static void llvm_printer_emit_function_type_params(llvm_printer* self, const tree_type* type)
{
        llvm_printc(self, '(');
        TREE_FOREACH_FUNC_TYPE_PARAM(type, it)
        {
                llvm_printer_emit_type(self, *it);
                if (it + 1 != tree_get_func_type_params_end(type))
                        llvm_prints(self, ", ");
        }
        if (tree_func_type_is_vararg(type))
                llvm_prints(self, ", ...");
        llvm_printc(self, ')');
}

static void llvm_printer_emit_function_type(llvm_printer* self, const tree_type* type)
{
        llvm_printer_emit_type(self, tree_get_func_type_result(type));
        llvm_print_wspace(self);
        llvm_printer_emit_function_type_params(self, type);
}

static void llvm_printer_emit_array_type(llvm_printer* self, const tree_type* type)
{
        if (tree_get_array_kind(type) != TAK_CONSTANT)
                return;

        llvm_printf(self, "[%u x ", tree_get_array_size(type));
        llvm_printer_emit_type(self, tree_get_array_eltype(type));
        llvm_printc(self, ']');
}

static void llvm_printer_emit_record_decl_name(llvm_printer* self, const tree_decl* rec)
{
        recmap_iter it;
        bool found = recmap_find(&self->rec_to_id, rec, &it);
        assert(found && "Id for record was not found.");
        llvm_printf(self, "%%record.%d", *recmap_iter_value(&it));
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
        else if (tree_declared_type_is(type, TDK_RECORD))
                llvm_printer_emit_record_decl_name(self, tree_get_decl_type_entity(type));
        
}

static void llvm_printer_emit_value_type(llvm_printer* self, const ssa_value* value)
{
        ssa_value_kind k = ssa_get_value_kind(value);
        if (k == SVK_LABEL)
                llvm_prints(self, "label");
        else
        {
                llvm_printer_emit_type(self, ssa_get_value_type(value));
                if (k == SVK_STRING)
                        llvm_printc(self, '*');
        }
}

static void llvm_printer_emit_constant(llvm_printer* self, const ssa_value* value)
{
        const avalue* val = ssa_get_constant_cvalue(value);
        if (avalue_is_zero(val) && tree_type_is_pointer(ssa_get_value_type(value)))
        {
                llvm_prints(self, "null");
                return;
        }

        char num[128];
        if (avalue_is_float(val))
                avalue_print_as_hex(val, num, ARRAY_SIZE(num));
        else
                avalue_print(val, num, ARRAY_SIZE(num), 0);
        llvm_prints(self, num);
}

extern void llvm_printer_emit_value(
        llvm_printer* self, const ssa_value* value, bool emit_type)
{
        if (emit_type)
        {
                llvm_printer_emit_value_type(self, value);
                llvm_print_wspace(self);
        }

        ssa_value_kind k = ssa_get_value_kind(value);
        if (k == SVK_LOCAL_VAR || k == SVK_PARAM)
                llvm_printf(self, "%%%u", ssa_get_value_id(value));
        else if (k == SVK_CONSTANT)
                llvm_printer_emit_constant(self, value);
        else if (k == SVK_GLOBAL_VAR || k == SVK_FUNCTION)
        {
                tree_decl* entity = k == SVK_FUNCTION
                        ? ssa_get_function_entity(value)
                        : ssa_get_global_var_entity(value);
                llvm_printf(self, "@%s", tree_get_id_string(self->tree, tree_get_decl_name(entity)));
        }
        else if (k == SVK_STRING)
                llvm_printf(self, "@.str.%u", ssa_get_value_id(value));
        else if (k == SVK_LABEL)
                llvm_printf(self, "%%%u", ssa_get_value_id(value));
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
        const tree_type* t = ssa_get_value_type(ssa_get_instr_operand_value(instr, 0));
        if (tree_type_is_floating(t))
                return LBIK_FLOATING;

        return tree_type_is_signed_integer(t) ? LBIK_SIGNED : LBIK_UNSIGNED;
}

static const char* llvm_binary_instr_table[SBIK_SIZE][LBIK_SIZE] =
{
        { NULL,        NULL,        NULL,  },
        { "mul nsw",   "mul",       "fmul", },
        { "sdiv",      "udiv",      "fdiv", },
        { "srem",      "urem",      "frem", },
        { "add nsw",   "add",       "fadd", },
        { "",          "",          "", }, // SBIK_PTRADD
        { "sub nsw",   "sub",       "fsub", },
        { "shl",       "shl",       NULL, },
        { "ashr",      "lshr",      NULL, },
        { "and",       "and",       NULL, },
        { "or",        "or",        NULL, },
        { "xor",       "xor",       NULL, },
        { "icmp slt",  "icmp ult",  "fcmp olt", },
        { "icmp sgt",  "icmp ugt",  "fcmp ogt", },
        { "icmp sle",  "icmp ule",  "fcmp ole", },
        { "icmp sge",  "icmp uge",  "fcmp oge", },
        { "icmp eq",   "icmp eq",   "fcmp oeq", },
        { "icmp ne",   "icmp ne",   "fcmp one", },
};

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
        size_t from_size = tree_get_sizeof(target, from);
        size_t to_size = tree_get_sizeof(target, to);
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

static void llvm_printer_emit_alloca_instr(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, var, false);
        llvm_prints(self, " = alloca ");
        llvm_printer_emit_type(self, ssa_get_allocated_type(instr));
}

static void llvm_printer_emit_volatile(llvm_printer* self, const tree_type* type)
{
        type = tree_desugar_ctype(type);
        if (!tree_type_is(type, TTK_POINTER))
                return;

        type = tree_get_pointer_target(type);
        if (tree_get_type_quals(type) & TTQ_VOLATILE)
                llvm_prints(self, "volatile ");
}

static void llvm_printer_emit_load_instr(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* result = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, result, false);
        llvm_prints(self, " = load ");
        ssa_value* from = ssa_get_instr_operand_value(instr, 0);
        llvm_printer_emit_volatile(self, ssa_get_value_type(from));
        llvm_printer_emit_type(self, ssa_get_value_type(result));
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, from, true);
}

static void llvm_printer_emit_cast_instr(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        const ssa_value* operand = ssa_get_instr_operand_value(instr, 0);
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

static void _llvm_printer_emit_binary_instr(llvm_printer* self, const ssa_instr* instr)
{
        ssa_value* first = ssa_get_instr_operand_value(instr, 0);
        ssa_value* second = ssa_get_instr_operand_value(instr, 1);
        ssa_binop_kind k = ssa_get_binop_kind(instr);
        bool emit_type_for_second_operand = false;

        if (k == SBIK_PTRADD)
        {
                llvm_prints(self, "getelementptr inbounds ");
                llvm_printer_emit_type(self,
                        tree_get_pointer_target(ssa_get_value_type(first)));
                llvm_prints(self, ", ");
                emit_type_for_second_operand = true;
        }
        else
                llvm_printf(self, "%s ",
                        llvm_binary_instr_table[k][llvm_get_binary_instr_kind(instr)]);

        llvm_printer_emit_value(self, first, true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, second, emit_type_for_second_operand);
}

static void llvm_printer_generate_tmp_var(llvm_printer* self, char* buffer)
{
        sprintf(buffer, "%%tmp.%u", self->tmp_id++);
}

static void llvm_printer_emit_cmp_instr(llvm_printer* self, const ssa_instr* instr)
{
        // since llvm cmp instruction yields i1
        // we need to implicitly cast it to a type specified by instruction variable
        char tmp[64];
        llvm_printer_generate_tmp_var(self, tmp);

        llvm_printf(self, "%s = ", tmp);
        _llvm_printer_emit_binary_instr(self, instr);
        llvm_print_endl(self);
        llvm_print_indent(self);

        const ssa_value* var = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, var, false);
        assert(tree_builtin_type_is(ssa_get_value_type(var), TBTK_INT32));
        llvm_printf(self, " = zext i1 %s to i32", tmp);
}

static void llvm_printer_emit_binary_instr(llvm_printer* self, const ssa_instr* instr)
{
        ssa_binop_kind k = ssa_get_binop_kind(instr);
        if (k >= SBIK_LE && k <= SBIK_NEQ)
        {
                llvm_printer_emit_cmp_instr(self, instr);
                return;
        }

        llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
        llvm_prints(self, " = ");
        _llvm_printer_emit_binary_instr(self, instr);
}

static void llvm_printer_emit_store_instr(llvm_printer* self, const ssa_instr* instr)
{
        llvm_prints(self, "store ");
        ssa_value* where = ssa_get_instr_operand_value(instr, 1);
        llvm_printer_emit_volatile(self, ssa_get_value_type(where));
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 0), true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 1), true);
}

static void llvm_printer_emit_getfieldaddr_instr(llvm_printer* self, const ssa_instr* instr)
{
        llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
        llvm_prints(self, " = getelementptr inbounds ");
        ssa_value* rec = ssa_get_instr_operand_value(instr, 0);
        llvm_printer_emit_type(self, tree_get_pointer_target(ssa_get_value_type(rec)));
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, rec, true);
        llvm_printf(self, ", i32 0, i32 %u", ssa_get_getfieldaddr_index(instr));
}

static void llvm_printer_emit_cc(llvm_printer* self, const tree_type* func)
{
        tree_calling_convention cc = tree_get_func_type_cc(func);
        if (cc == TCC_STDCALL && self->ssa->target->kind == TTAK_X86_32)
                llvm_prints(self, "x86_stdcallcc ");
}

static void llvm_printer_emit_call_instr(llvm_printer* self, const ssa_instr* instr)
{
        if (ssa_instr_has_var(instr))
        {
                llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
                llvm_prints(self, " = ");
        }
        llvm_prints(self, "call ");

        ssa_value* func = ssa_get_called_func(instr);
        tree_type* func_type = tree_desugar_type(tree_get_pointer_target(ssa_get_value_type(func)));
        llvm_printer_emit_cc(self, func_type);

        llvm_printer_emit_type(self, tree_get_pointer_target(ssa_get_value_type(func)));
        llvm_print_wspace(self);
        llvm_printer_emit_value(self, ssa_get_called_func(instr), false);

        llvm_printc(self, '(');
        for (ssa_value_use* it = ssa_get_instr_operands_begin(instr) + 1,
                *end = ssa_get_instr_operands_end(instr); 
                it != end; it++)
        {
                llvm_printer_emit_value(self, ssa_get_value_use_value(it), true);
                if (it + 1 != end)
                        llvm_prints(self, ", ");
        }
        llvm_printc(self, ')');
}

static void llvm_printer_emit_phi_instr(llvm_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, var, false);
        llvm_prints(self, " = phi ");
        llvm_printer_emit_type(self, ssa_get_value_type(var));
        llvm_print_wspace(self);

        int i = 0;
        SSA_FOREACH_INSTR_OPERAND(instr, it, end)
        {
                bool even = i % 2 == 0;
                if (even)
                        llvm_printc(self, '[');
                llvm_printer_emit_value(self, ssa_get_value_use_value(it), false);
                if (!even)
                        llvm_printc(self, ']');
                if (it + 1 != end)
                        llvm_prints(self, ", ");
                i++;
        }
}

static void llvm_printer_emit_conditional_jump(llvm_printer* self, const ssa_instr* instr)
{
        char tmp[64];
        llvm_printer_generate_tmp_var(self, tmp);

        ssa_value* cond = ssa_get_instr_operand_value(instr, 0);
        assert(tree_builtin_type_is(ssa_get_value_type(cond), TBTK_INT32));
        llvm_printf(self, "%s = trunc ", tmp);
        llvm_printer_emit_value(self, cond, true);
        llvm_prints(self, " to i1");
        llvm_print_endl(self);
        llvm_print_indent(self);

        llvm_printf(self, "br i1 %s, label ", tmp);
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 1), false);
        llvm_prints(self, ", label ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 2), false);
}

static void llvm_printer_emit_inderect_jump(llvm_printer* self, const ssa_instr* instr)
{
        llvm_prints(self, "br label ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 0), false);
}

static void llvm_printer_emit_return(llvm_printer* self, const ssa_instr* instr)
{
        llvm_prints(self, "ret ");
        if (ssa_get_instr_operands_size(instr))
                llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 0), true);
        else
                llvm_prints(self, "void");
}

static void llvm_printer_emit_switch(llvm_printer* self, const ssa_instr* instr)
{
        llvm_prints(self, "switch ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 0), true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 1), true);
        llvm_prints(self, " [");

        self->indent_lvl++;
        size_t num_ops = ssa_get_instr_operands_size(instr);
        for (size_t i = 2; i < num_ops; i += 2)
        {
                llvm_print_endl(self);
                llvm_print_indent(self);
                llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, i), true);
                llvm_prints(self, ", ");
                llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, i + 1), true);
        }
        self->indent_lvl--;
        llvm_print_endl(self);
        llvm_print_indent(self);
        llvm_printc(self, ']');
}

static void llvm_printer_emit_terminator_instr(llvm_printer* self, const ssa_instr* instr)
{
        ssa_terminator_instr_kind k = ssa_get_terminator_instr_kind(instr);
        if (k == STIK_CONDITIONAL_JUMP)
                llvm_printer_emit_conditional_jump(self, instr);
        else if (k == STIK_INDERECT_JUMP)
                llvm_printer_emit_inderect_jump(self, instr);
        else if (k == STIK_RETURN)
                llvm_printer_emit_return(self, instr);
        else if (k == STIK_SWITCH)
                llvm_printer_emit_switch(self, instr);
}

static void llvm_printer_emit_ordering(llvm_printer* self, ssa_memorder_kind k)
{
        if (k == SMK_UNORDERED)
                llvm_prints(self, "unordered ");
        else if (k == SMK_MONOTONIC)
                llvm_prints(self, "monotonic ");
        else if (k == SMK_ACQUIRE)
                llvm_prints(self, "acquire ");
        else if (k == SMK_RELEASE)
                llvm_prints(self, "release ");
        else if (k == SMK_ACQ_REL)
                llvm_prints(self, "acq_rel ");
        else if (k == SMK_SEQ_CST)
                llvm_prints(self, "seq_cst ");
}

static void llvm_printer_emit_syncscope(llvm_printer* self, ssa_syncscope_kind k)
{
        if (k == SSK_SINGLE_THREAD)
                llvm_prints(self, "syncscope(\"singlethread\") ");
}

static const char* llvm_atomicrmw_instr_table[] = 
{
        "",
        "add",
        "xchg",
};

static void llvm_printer_emit_atomicrmw_instr(llvm_printer* self, const ssa_instr* instr)
{
        llvm_printer_emit_value(self, ssa_get_instr_cvar(instr), false);
        llvm_prints(self, " = atomicrmw ");
        ssa_value* pointer = ssa_get_instr_operand_value(instr, 0);
        llvm_printer_emit_volatile(self, ssa_get_value_type(pointer));
        llvm_prints(self, llvm_atomicrmw_instr_table[ssa_get_atomic_rmw_instr_kind(instr)]);
        llvm_printc(self, ' ');
        llvm_printer_emit_value(self, pointer, true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 1), true);
        llvm_printc(self, ' ');
        llvm_printer_emit_ordering(self, ssa_get_atomic_rmw_instr_ordering(instr));
}

static void llvm_printer_emit_fence_instr(llvm_printer* self, const ssa_instr* instr)
{
        llvm_prints(self, "fence ");
        if (ssa_get_fence_instr_syncscope(instr) == SSK_SINGLE_THREAD)
                llvm_prints(self, "syncscope(\"singlethread\") ");
        llvm_printer_emit_ordering(self, ssa_get_fence_instr_ordering(instr));
}

static void llvm_printer_emit_cmpxchg_instr(llvm_printer* self, const ssa_instr* instr)
{
        char result[64];
        llvm_printer_generate_tmp_var(self, result);
        llvm_printf(self, "%s = cmpxchg weak ", result);
        ssa_value* pointer = ssa_get_instr_operand_value(instr, 0);
        llvm_printer_emit_volatile(self, ssa_get_value_type(pointer));
        llvm_printer_emit_value(self, pointer, true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 1), true);
        llvm_prints(self, ", ");
        llvm_printer_emit_value(self, ssa_get_instr_operand_value(instr, 2), true);
        llvm_print_wspace(self);
        llvm_printer_emit_ordering(self, ssa_get_atomic_cmpxchg_instr_success_ordering(instr));
        llvm_printer_emit_ordering(self, ssa_get_atomic_cmpxchg_instr_failure_ordering(instr));
        llvm_print_endl(self);
        llvm_print_indent(self);

        char loaded[64];
        llvm_printer_generate_tmp_var(self, loaded);
        llvm_printf(self, "%s = extractvalue { i32, i1 } %s, 1", loaded, result);
        llvm_print_endl(self);
        llvm_print_indent(self);

        const ssa_value* var = ssa_get_instr_cvar(instr);
        llvm_printer_emit_value(self, var, false);
        assert(tree_builtin_type_is(ssa_get_value_type(var), TBTK_INT32));
        llvm_printf(self, " = zext i1 %s to i32", loaded);
}

extern void llvm_printer_emit_instr(llvm_printer* self, const ssa_instr* instr)
{
        llvm_print_endl(self);
        llvm_print_indent(self);

        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_ALLOCA)
                llvm_printer_emit_alloca_instr(self, instr);
        else if (k == SIK_LOAD)
                llvm_printer_emit_load_instr(self, instr);
        else if (k == SIK_CAST)
                llvm_printer_emit_cast_instr(self, instr);
        else if (k == SIK_BINARY)
                llvm_printer_emit_binary_instr(self, instr);
        else if (k == SIK_STORE)
                llvm_printer_emit_store_instr(self, instr);
        else if (k == SIK_GETFIELDADDR)
                llvm_printer_emit_getfieldaddr_instr(self, instr);
        else if (k == SIK_CALL)
                llvm_printer_emit_call_instr(self, instr);
        else if (k == SIK_PHI)
                llvm_printer_emit_phi_instr(self, instr);
        else if (k == SIK_TERMINATOR)
                llvm_printer_emit_terminator_instr(self, instr);
        else if (k == SIK_ATOMIC_RMW)
                llvm_printer_emit_atomicrmw_instr(self, instr);
        else if (k == SIK_FENCE)
                llvm_printer_emit_fence_instr(self, instr);
        else if (k == SIK_ATOMIC_CMPXCHG)
                llvm_printer_emit_cmpxchg_instr(self, instr);
}

extern void llvm_printer_emit_block(llvm_printer* self, const ssa_block* block)
{
        llvm_print_endl(self);
        llvm_printf(self, "; <label>:%u", ssa_get_value_id(ssa_get_block_clabel(block)));
        SSA_FOREACH_BLOCK_INSTR(block, instr)
                llvm_printer_emit_instr(self, instr);
        llvm_print_endl(self);
}

static void llvm_printer_emit_struct_fields(llvm_printer* self, const tree_decl* rec)
{
        const tree_decl_scope* fields = tree_get_record_cfields(rec);
        bool first = true;
        TREE_FOREACH_DECL_IN_SCOPE(fields, field)
        {
                if (!tree_decl_is(field, TDK_FIELD))
                        continue;

                if (!first)
                        llvm_prints(self, ", ");
                first = false;

                llvm_printer_emit_type(self, tree_get_decl_type(field));
        }
}

static void llvm_printer_emit_union_fields(llvm_printer* self, const tree_decl* rec)
{
        const tree_decl_scope* fields = tree_get_record_cfields(rec);
        tree_type* largest_type = NULL;

        TREE_FOREACH_DECL_IN_SCOPE(fields, field)
        {
                if (!tree_decl_is(field, TDK_FIELD))
                        continue;

                tree_type* field_type = tree_get_decl_type(field);
                if (!largest_type || tree_get_sizeof(self->tree->target, field_type)
                        > tree_get_sizeof(self->tree->target, largest_type))
                {
                        largest_type = field_type;
                }
        }

        llvm_printer_emit_type(self, largest_type);
}

static void llvm_printer_emit_record_fields(llvm_printer* self, const tree_decl* rec)
{
        llvm_prints(self, "<{ ");
        if (tree_record_is_union(rec))
                llvm_printer_emit_union_fields(self, rec);
        else
                llvm_printer_emit_struct_fields(self, rec);
        llvm_prints(self, " }>");
}

static void llvm_printer_emit_record(llvm_printer* self, const tree_decl* rec)
{
        recmap_iter it;
        if (recmap_find(&self->rec_to_id, rec, &it))
                return;

        recmap_insert(&self->rec_to_id, rec, self->rec_id++);

        llvm_print_endl(self);
        llvm_printer_emit_record_decl_name(self, rec);
        llvm_prints(self, " = type ");
        llvm_printer_emit_record_fields(self, rec);
}

static void llvm_printer_emit_string(llvm_printer* self, const ssa_value* val)
{
        llvm_print_endl(self);
        llvm_printer_emit_value(self, val, false);
        llvm_prints(self, " = private constant ");
        llvm_printer_emit_type(self, ssa_get_value_type(val));

        strentry entry;
        if (!tree_get_id_strentry(self->tree, ssa_get_string_value(val), &entry))
                return;

        llvm_prints(self, " c\"");
        for (size_t i = 0; i < entry.size; i++)
        {
                int c = entry.data[i];
                llvm_printf(self, (isprint(c) ? "%c" : "\\%02X"), c);

        }
        llvm_printc(self, '"');
}

static void llvm_printer_emit_dll_storage_class(llvm_printer* self, const tree_decl* decl)
{
        if (tree_get_decl_dll_storage_class(decl) == TDSC_IMPORT)
                llvm_prints(self, "dllimport ");
}

static void llvm_printer_emit_linkage(llvm_printer* self, const tree_decl* decl)
{
        tree_storage_class sc = tree_get_decl_storage_class(decl);
        if (sc == TSC_STATIC)
                llvm_prints(self, "internal ");

        if (tree_decl_is(decl, TDK_FUNCTION))
                return;

        if (sc == TSC_EXTERN)
                llvm_prints(self, "external ");

        if (tree_get_decl_storage_duration(decl) == TSD_THREAD)
                llvm_prints(self, "thread_local ");

}

static void llvm_printer_emit_function(llvm_printer* self, const ssa_value* val)
{
        self->tmp_id = 0;
        llvm_print_endl(self);
        bool defined = ssa_get_function_blocks_begin(val) != ssa_get_function_blocks_cend(val);

        tree_decl* func = ssa_get_function_entity(val);
        tree_type* func_type = tree_get_decl_type(func);
        llvm_prints(self, defined ? "define " : "declare ");
        llvm_printer_emit_linkage(self, func);
        llvm_printer_emit_dll_storage_class(self, func);
        llvm_printer_emit_cc(self, func_type);
        llvm_printer_emit_type(self, tree_get_func_type_result(func_type));
        llvm_printf(self, " @%s", tree_get_id_string(self->tree, tree_get_decl_name(func)));
        llvm_printer_emit_function_type_params(self, func_type);

        if (!defined)
                return;

        llvm_print_endl(self);
        llvm_printc(self, '{');
        self->indent_lvl++;
        SSA_FOREACH_FUNCTION_BLOCK(val, block)
                llvm_printer_emit_block(self, block);
        self->indent_lvl--;
        llvm_print_endl(self);
        llvm_printc(self, '}');
        llvm_print_endl(self);
}

static void llvm_printer_emit_global_var(llvm_printer* self, const ssa_value* val)
{
        llvm_print_endl(self);
        tree_decl* decl = ssa_get_global_var_entity(val);
        llvm_printer_emit_value(self, val, false);
        llvm_prints(self, " = ");
        llvm_printer_emit_linkage(self, decl);
        llvm_prints(self, "global ");
        llvm_printer_emit_type(self, tree_get_decl_type(decl));
        if (tree_get_decl_storage_class(decl) != TSC_EXTERN)
                llvm_prints(self, " zeroinitializer");
}

static void llvm_printer_emit_global_value(llvm_printer* self, const ssa_value* val)
{
        switch (ssa_get_value_kind(val))
        {
                case SVK_STRING:
                        llvm_printer_emit_string(self, val);
                        return;
                case SVK_GLOBAL_VAR:
                        llvm_printer_emit_global_var(self, val);
                        return;
                case SVK_FUNCTION:
                        llvm_printer_emit_function(self, val);
                        return;
                default:
                        return;
        }
}

extern void llvm_printer_emit_module(llvm_printer* self, const ssa_module* module)
{
        SSA_FOREACH_MODULE_TYPE_DECL(module, it, end)
                if (tree_decl_is(*it, TDK_RECORD))
                        llvm_printer_emit_record(self, *it);
        SSA_FOREACH_MODULE_GLOBAL(module, it, end)
                llvm_printer_emit_global_value(self, *it);
}