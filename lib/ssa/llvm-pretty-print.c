#include "scc/core/num.h"
#include "scc/ssa/const.h"
#include "scc/ssa/pretty-print.h"
#include "scc/ssa/value.h"
#include "scc/ssa/module.h"
#include "scc/ssa/instr.h"
#include "scc/ssa/context.h"
#include "scc/ssa/block.h"
#include "printer.h"
#include "scc/tree/type.h"
#include <ctype.h>

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

static void ssa_print_type(ssa_printer*, const tree_type*);

static void ssa_print_func_type_params(ssa_printer* self, const tree_type* type)
{
        ssa_printc(self, '(');
        if (tree_get_semantic_func_type_params_size(type))
                TREE_FOREACH_FUNC_TYPE_PARAM(type, it)
                {
                        ssa_print_type(self, *it);
                        if (it + 1 != tree_get_func_type_params_end(type))
                                ssa_prints(self, ", ");
                }
        if (tree_func_type_is_vararg(type))
                ssa_prints(self, ", ...");
        ssa_printc(self, ')');
}

static void ssa_print_record_name(ssa_printer* self, const tree_decl* rec)
{
        ssa_prints(self, "%record.");
        ssa_print_record_id(self, rec);
}

static void _ssa_print_type(ssa_printer* self, const tree_type* type, int print_void)
{
        type = tree_desugar_type_c(type);
        tree_type* target;
        tree_builtin_type_kind btk;
        switch (tree_get_type_kind(type))
        {
                case TTK_BUILTIN:
                        btk = tree_get_builtin_type_kind(type);
                        if (btk == TBTK_VOID && !print_void)
                                btk = TBTK_INT8;
                        ssa_prints(self, llvm_builtin_type_table[btk]);
                        return;
                case TTK_FUNCTION:
                        _ssa_print_type(self, tree_get_func_type_result(type), 1);
                        ssa_printc(self, ' ');
                        ssa_print_func_type_params(self, type);
                        return;
                case TTK_POINTER:
                        target = tree_get_pointer_target(type);
                        if (tree_type_is_void(target))
                                ssa_prints(self, llvm_builtin_type_table[TBTK_INT8]);
                        else
                                ssa_print_type(self, target);
                        ssa_printc(self, '*');
                        return;
                case TTK_ARRAY:
                        unsigned size = tree_array_is(type, TAK_CONSTANT)
                                ? tree_get_array_size(type) : 0;
                        ssa_printf(self, "[%u x ", size);
                        ssa_print_type(self, tree_get_array_eltype(type));
                        ssa_printc(self, ']');
                        return;
                default:
                        if (tree_declared_type_is(type, TDK_RECORD))
                                ssa_print_record_name(self, tree_get_decl_type_entity(type));
        }
}

static void ssa_print_type(ssa_printer* self, const tree_type* type)
{
        _ssa_print_type(self, type, 0);
}

static void ssa_print_value_type(ssa_printer* self, const ssa_value* value)
{
        ssa_value_kind k = ssa_get_value_kind(value);
        if (k == SVK_LABEL)
                ssa_prints(self, "label");
        else
        {
                ssa_print_type(self, ssa_get_value_type(value));
                // todo: VVVVV
                if (k == SVK_STRING)
                        ssa_printc(self, '*');
        }
}

static void ssa_print_constant_value(ssa_printer* self, tree_type* type, const struct num* val)
{
        if (num_is_zero(val) && tree_type_is_pointer(type))
        {
                ssa_prints(self, "null");
                return;
        }

        char num[128];
        if (val->kind == NUM_FLOAT)
                num_to_hex(val, num, ARRAY_SIZE(num));
        else
                num_to_str(val, num, ARRAY_SIZE(num));
        ssa_prints(self, num);
}

static void ssa_print_value(ssa_printer* self, const ssa_value* value, bool print_type)
{
        if (print_type)
        {
                ssa_print_value_type(self, value);
                ssa_printc(self, ' ');
        }

        ssa_value_kind k = ssa_get_value_kind(value);
        if (k == SVK_LOCAL_VAR || k == SVK_PARAM)
                ssa_printf(self, "%%%u", ssa_get_value_id(value));
        else if (k == SVK_CONSTANT)
                ssa_print_constant_value(self, 
                        ssa_get_value_type(value), ssa_get_constant_cvalue(value));
        else if (k == SVK_GLOBAL_VAR || k == SVK_FUNCTION)
        {
                tree_decl* entity = k == SVK_FUNCTION
                        ? ssa_get_function_entity(value)
                        : ssa_get_global_var_entity(value);
                ssa_printc(self, '@');
                ssa_print_tree_id(self, tree_get_decl_name(entity));
        }
        else if (k == SVK_STRING)
                ssa_printf(self, "@.str.%u", ssa_get_value_id(value));
        else if (k == SVK_LABEL)
                ssa_printf(self, "%%%u", ssa_get_value_id(value));
        else if (k == SVK_UNDEF)
                ssa_prints(self, "undef");
}

typedef enum
{
        LBIK_SIGNED,
        LBIK_UNSIGNED,
        LBIK_FLOATING,
        LBIK_SIZE
} llvm_binary_instr_kind;

static llvm_binary_instr_kind ssa_get_llvm_binary_instr_kind(const ssa_instr* instr)
{
        const tree_type* t = ssa_get_value_type(ssa_get_instr_operand_value(instr, 0));
        if (tree_type_is_floating(t))
                return LBIK_FLOATING;

        return tree_type_is_signed_integer(t) ? LBIK_SIGNED : LBIK_UNSIGNED;
}

static const char* llvm_binary_instr_table[SBIK_SIZE][LBIK_SIZE] =
{
        { NULL,        NULL,        NULL, },
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

static llvm_cast_kind ssa_get_llvm_cast_kind(const tree_target_info* target,
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

static void ssa_print_alloca_instr(ssa_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        ssa_print_value(self, var, false);
        ssa_prints(self, " = alloca ");
        ssa_print_type(self, ssa_get_allocated_type(instr));
        ssa_prints(self, ", align 4");
}

static void ssa_print_volatile(ssa_printer* self, const tree_type* type)
{
        type = tree_desugar_type_c(type);
        if (!tree_type_is(type, TTK_POINTER))
                return;

        type = tree_get_pointer_target(type);
        if (tree_get_type_quals(type) & TTQ_VOLATILE)
                ssa_prints(self, "volatile ");
}

static void ssa_print_load_instr(ssa_printer* self, const ssa_instr* instr)
{
        const ssa_value* result = ssa_get_instr_cvar(instr);
        ssa_print_value(self, result, false);
        ssa_prints(self, " = load ");
        ssa_value* from = ssa_get_instr_operand_value(instr, 0);
        ssa_print_volatile(self, ssa_get_value_type(from));
        ssa_print_type(self, ssa_get_value_type(result));
        ssa_prints(self, ", ");
        ssa_print_value(self, from, true);
}

static void ssa_print_cast_instr(ssa_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        const ssa_value* operand = ssa_get_instr_operand_value(instr, 0);
        ssa_print_value(self, var, false);
        ssa_prints(self, " = ");

        tree_type* to = ssa_get_value_type(var);
        llvm_cast_kind kind = ssa_get_llvm_cast_kind(
                self->context->target, ssa_get_value_type(operand), to);
        if (kind == LSK_INVALID)
                return;

        ssa_printf(self, "%s ", llvm_cast_table[kind]);
        ssa_print_value(self, operand, true);
        ssa_prints(self, " to ");
        ssa_print_type(self, to);
}

static void _ssa_print_binary_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_value* first = ssa_get_instr_operand_value(instr, 0);
        ssa_value* second = ssa_get_instr_operand_value(instr, 1);
        ssa_binop_kind k = ssa_get_binop_kind(instr);
        bool print_type_for_second_operand = false;

        if (k == SBIK_PTRADD)
        {
                ssa_prints(self, "getelementptr inbounds ");
                ssa_print_type(self, tree_get_pointer_target(ssa_get_value_type(first)));
                ssa_prints(self, ", ");
                print_type_for_second_operand = true;
        }
        else
                ssa_printf(self, "%s ", llvm_binary_instr_table[k][ssa_get_llvm_binary_instr_kind(instr)]);

        ssa_print_value(self, first, true);
        ssa_prints(self, ", ");
        ssa_print_value(self, second, print_type_for_second_operand);
}

static void ssa_print_cmp_instr(ssa_printer*self, const ssa_instr* instr)
{
        // since llvm cmp instruction yields i1
        // we need to implicitly cast it to a type specified by instruction variable
        char tmp[64];
        ssa_gen_tmp_var(self, tmp);

        ssa_printf(self, "%s = ", tmp);
        _ssa_print_binary_instr(self, instr);
        ssa_printc(self, '\n');
        ssa_print_indent(self);

        const ssa_value* var = ssa_get_instr_cvar(instr);
        ssa_print_value(self, var, false);
        assert(tree_builtin_type_is(ssa_get_value_type(var), TBTK_INT32));
        ssa_printf(self, " = zext i1 %s to i32", tmp);
}

static void ssa_print_binary_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_binop_kind k = ssa_get_binop_kind(instr);
        if (k >= SBIK_LE && k <= SBIK_NEQ)
        {
                ssa_print_cmp_instr(self, instr);
                return;
        }

        ssa_print_value(self, ssa_get_instr_cvar(instr), false);
        ssa_prints(self, " = ");
        _ssa_print_binary_instr(self, instr);
}

static void ssa_print_store_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_prints(self, "store ");
        ssa_value* where = ssa_get_instr_operand_value(instr, 1);
        ssa_print_volatile(self, ssa_get_value_type(where));
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 0), true);
        ssa_prints(self, ", ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 1), true);
}

static void ssa_print_getfieldaddr_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value(self, ssa_get_instr_cvar(instr), false);
        ssa_prints(self, " = getelementptr inbounds ");
        ssa_value* rec = ssa_get_instr_operand_value(instr, 0);
        ssa_print_type(self, tree_get_pointer_target(ssa_get_value_type(rec)));
        ssa_prints(self, ", ");
        ssa_print_value(self, rec, true);
        ssa_printf(self, ", i32 0, i32 %u", ssa_get_getfieldaddr_index(instr));
}


static void ssa_print_cc(ssa_printer* self, const tree_type* func)
{
        tree_calling_convention cc = tree_get_func_type_cc(func);
        if (cc == TCC_STDCALL && self->context->target->kind == TTAK_X86_32)
                ssa_prints(self, "x86_stdcallcc ");
}

static void ssa_print_call_instr(ssa_printer* self, const ssa_instr* instr)
{
        if (ssa_instr_has_var(instr))
        {
                ssa_print_value(self, ssa_get_instr_cvar(instr), false);
                ssa_prints(self, " = ");
        }
        ssa_prints(self, "call ");

        ssa_value* func = ssa_get_called_func(instr);
        tree_type* func_type = tree_desugar_type(
                tree_get_pointer_target(tree_desugar_type(ssa_get_value_type(func))));
        ssa_print_cc(self, func_type);

        ssa_print_type(self, func_type);
        ssa_printc(self, ' ');
        ssa_print_value(self, ssa_get_called_func(instr), false);

        ssa_printc(self, '(');
        for (ssa_value_use* it = ssa_get_instr_operands_begin(instr) + 1,
                *end = ssa_get_instr_operands_end(instr);
                it != end; it++)
        {
                ssa_print_value(self, ssa_get_value_use_value(it), true);
                if (it + 1 != end)
                        ssa_prints(self, ", ");
        }
        ssa_printc(self, ')');
}

static void ssa_print_phi_instr(ssa_printer* self, const ssa_instr* instr)
{
        const ssa_value* var = ssa_get_instr_cvar(instr);
        ssa_print_value(self, var, false);
        ssa_prints(self, " = phi ");
        ssa_print_type(self, ssa_get_value_type(var));
        ssa_printc(self, ' ');

        int i = 0;
        SSA_FOREACH_INSTR_OPERAND(instr, it, end)
        {
                bool even = i % 2 == 0;
                if (even)
                        ssa_printc(self, '[');
                ssa_print_value(self, ssa_get_value_use_value(it), false);
                if (!even)
                        ssa_printc(self, ']');
                if (it + 1 != end)
                        ssa_prints(self, ", ");
                i++;
        }
}

static void ssa_print_cond_jmp_instr(ssa_printer* self, const ssa_instr* instr)
{
        char tmp[64];
        ssa_gen_tmp_var(self, tmp);

        ssa_value* cond = ssa_get_instr_operand_value(instr, 0);
        assert(tree_builtin_type_is(ssa_get_value_type(cond), TBTK_INT32));
        ssa_printf(self, "%s = trunc ", tmp);
        ssa_print_value(self, cond, true);
        ssa_prints(self, " to i1");
        ssa_printc(self, '\n');
        ssa_print_indent(self);

        ssa_printf(self, "br i1 %s, label ", tmp);
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 1), false);
        ssa_prints(self, ", label ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 2), false);
}

static void ssa_print_inderect_jmp_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_prints(self, "br label ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 0), false);
}

static void ssa_print_return_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_prints(self, "ret ");
        if (ssa_get_instr_operands_size(instr))
                ssa_print_value(self, ssa_get_instr_operand_value(instr, 0), true);
        else
                ssa_prints(self, "void");
}

static void ssa_print_switch_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_prints(self, "switch ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 0), true);
        ssa_prints(self, ", ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 1), true);
        ssa_prints(self, " [");

        self->indent_lvl++;
        size_t num_ops = ssa_get_instr_operands_size(instr);
        for (size_t i = 2; i < num_ops; i += 2)
        {
                ssa_printc(self, '\n');
                ssa_print_indent(self);
                ssa_print_value(self, ssa_get_instr_operand_value(instr, i), true);
                ssa_prints(self, ", ");
                ssa_print_value(self, ssa_get_instr_operand_value(instr, i + 1), true);
        }
        self->indent_lvl--;
        ssa_printc(self, '\n');
        ssa_print_indent(self);
        ssa_printc(self, ']');
}

static void ssa_print_terminator_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_terminator_instr_kind k = ssa_get_terminator_instr_kind(instr);
        if (k == STIK_CONDITIONAL_JUMP)
                ssa_print_cond_jmp_instr(self, instr);
        else if (k == STIK_INDERECT_JUMP)
                ssa_print_inderect_jmp_instr(self, instr);
        else if (k == STIK_RETURN)
                ssa_print_return_instr(self, instr);
        else if (k == STIK_SWITCH)
                ssa_print_switch_instr(self, instr);
}

static void ssa_print_ordering(ssa_printer* self, ssa_memorder_kind k)
{
        if (k == SMK_UNORDERED)
                ssa_prints(self, "unordered ");
        else if (k == SMK_MONOTONIC)
                ssa_prints(self, "monotonic ");
        else if (k == SMK_ACQUIRE)
                ssa_prints(self, "acquire ");
        else if (k == SMK_RELEASE)
                ssa_prints(self, "release ");
        else if (k == SMK_ACQ_REL)
                ssa_prints(self, "acq_rel ");
        else if (k == SMK_SEQ_CST)
                ssa_prints(self, "seq_cst ");
}

static void ssa_print_syncscope(ssa_printer* self, ssa_syncscope_kind k)
{
        if (k == SSK_SINGLE_THREAD)
                ssa_prints(self, "syncscope(\"singlethread\") ");
}

static const char* llvm_atomicrmw_instr_table[] =
{
        "",
        "add",
        "xchg",
};

static void ssa_print_atomicrmw_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_print_value(self, ssa_get_instr_cvar(instr), false);
        ssa_prints(self, " = atomicrmw ");
        ssa_value* pointer = ssa_get_instr_operand_value(instr, 0);
        ssa_print_volatile(self, ssa_get_value_type(pointer));
        ssa_prints(self, llvm_atomicrmw_instr_table[ssa_get_atomic_rmw_instr_kind(instr)]);
        ssa_printc(self, ' ');
        ssa_print_value(self, pointer, true);
        ssa_prints(self, ", ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 1), true);
        ssa_printc(self, ' ');
        ssa_print_ordering(self, ssa_get_atomic_rmw_instr_ordering(instr));
}

static void ssa_print_fence_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_prints(self, "fence ");
        if (ssa_get_fence_instr_syncscope(instr) == SSK_SINGLE_THREAD)
                ssa_prints(self, "syncscope(\"singlethread\") ");
        ssa_print_ordering(self, ssa_get_fence_instr_ordering(instr));
}

static void ssa_print_cmpxchg_instr(ssa_printer* self, const ssa_instr* instr)
{
        char result[64];
        ssa_gen_tmp_var(self, result);
        ssa_printf(self, "%s = cmpxchg weak ", result);
        ssa_value* pointer = ssa_get_instr_operand_value(instr, 0);
        ssa_print_volatile(self, ssa_get_value_type(pointer));
        ssa_print_value(self, pointer, true);
        ssa_prints(self, ", ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 1), true);
        ssa_prints(self, ", ");
        ssa_print_value(self, ssa_get_instr_operand_value(instr, 2), true);
        ssa_printc(self, ' ');
        ssa_print_ordering(self, ssa_get_atomic_cmpxchg_instr_success_ordering(instr));
        ssa_print_ordering(self, ssa_get_atomic_cmpxchg_instr_failure_ordering(instr));
        ssa_printc(self, '\n');
        ssa_print_indent(self);

        char loaded[64];
        ssa_gen_tmp_var(self, loaded);
        ssa_printf(self, "%s = extractvalue { i32, i1 } %s, 1", loaded, result);
        ssa_printc(self, '\n');
        ssa_print_indent(self);

        const ssa_value* var = ssa_get_instr_cvar(instr);
        ssa_print_value(self, var, false);
        assert(tree_builtin_type_is(ssa_get_value_type(var), TBTK_INT32));
        ssa_printf(self, " = zext i1 %s to i32", loaded);
}

extern void ssa_print_instr(ssa_printer* self, const ssa_instr* instr)
{
        ssa_instr_kind k = ssa_get_instr_kind(instr);
        if (k == SIK_ALLOCA)
                ssa_print_alloca_instr(self, instr);
        else if (k == SIK_LOAD)
                ssa_print_load_instr(self, instr);
        else if (k == SIK_CAST)
                ssa_print_cast_instr(self, instr);
        else if (k == SIK_BINARY)
                ssa_print_binary_instr(self, instr);
        else if (k == SIK_STORE)
                ssa_print_store_instr(self, instr);
        else if (k == SIK_GETFIELDADDR)
                ssa_print_getfieldaddr_instr(self, instr);
        else if (k == SIK_CALL)
                ssa_print_call_instr(self, instr);
        else if (k == SIK_PHI)
                ssa_print_phi_instr(self, instr);
        else if (k == SIK_TERMINATOR)
                ssa_print_terminator_instr(self, instr);
        else if (k == SIK_ATOMIC_RMW)
                ssa_print_atomicrmw_instr(self, instr);
        else if (k == SIK_FENCE)
                ssa_print_fence_instr(self, instr);
        else if (k == SIK_ATOMIC_CMPXCHG)
                ssa_print_cmpxchg_instr(self, instr);
}

extern void ssa_print_block(ssa_printer* self, const ssa_block* block)
{
        ssa_printf(self, "; <label>:%u", ssa_get_value_id(ssa_get_block_clabel(block)));
        SSA_FOREACH_BLOCK_INSTR(block, instr)
        {
                ssa_printc(self, '\n');
                ssa_print_indent(self);
                ssa_print_instr(self, instr);
        }
}

static void ssa_print_struct_fields(ssa_printer* self, const tree_decl* rec)
{
        const tree_decl_scope* fields = tree_get_record_cfields(rec);
        bool first = true;
        TREE_FOREACH_DECL_IN_SCOPE(fields, field)
        {
                if (!tree_decl_is(field, TDK_FIELD))
                        continue;

                if (!first)
                        ssa_prints(self, ", ");
                first = false;

                ssa_print_type(self, tree_get_decl_type(field));
        }
}

static void ssa_print_union_fields(ssa_printer* self, const tree_decl* rec)
{
        const tree_decl_scope* fields = tree_get_record_cfields(rec);
        tree_type* largest_type = NULL;

        TREE_FOREACH_DECL_IN_SCOPE(fields, field)
        {
                if (!tree_decl_is(field, TDK_FIELD))
                        continue;

                tree_type* field_type = tree_get_decl_type(field);
                if (!largest_type || tree_get_sizeof(self->context->target, field_type)
                        > tree_get_sizeof(self->context->target, largest_type))
                {
                        largest_type = field_type;
                }
        }

        ssa_print_type(self, largest_type);
}

static void ssa_print_record_fields(ssa_printer* self, const tree_decl* rec)
{
        ssa_prints(self, "<{ ");
        if (tree_record_is_union(rec))
                ssa_print_union_fields(self, rec);
        else
                ssa_print_struct_fields(self, rec);
        ssa_prints(self, " }>");
}

static void ssa_print_record_decl(ssa_printer* self, const tree_decl* rec)
{
        ssa_print_record_name(self, rec);
        ssa_prints(self, " = type ");
        ssa_print_record_fields(self, rec);
}

static void ssa_print_string_value(ssa_printer* self, const ssa_value* val)
{
        ssa_print_value(self, val, false);
        ssa_prints(self, " = private constant ");
        ssa_print_type(self, ssa_get_value_type(val));
        
        struct strentry* entry = tree_get_id_strentry(self->context->tree, ssa_get_string_value(val));
        if (!entry)
                return;

        ssa_prints(self, " c\"");
        for (size_t i = 0; i < entry->size; i++)
        {
                int c = entry->data[i];
                ssa_printf(self, (isprint(c) ? "%c" : "\\%02X"), c);

        }
        ssa_printc(self, '"');
}

static void ssa_print_dll_storage_class(ssa_printer* self, const tree_decl* decl)
{
        if (tree_get_decl_dll_storage_class(decl) == TDSC_IMPORT)
                ssa_prints(self, "dllimport ");
}

static void ssa_print_linkage(ssa_printer* self, const tree_decl* decl)
{
        tree_storage_class sc = tree_get_decl_storage_class(decl);
        if (sc == TSC_STATIC)
                ssa_prints(self, "internal ");

        if (tree_decl_is(decl, TDK_FUNCTION))
                return;

        if (sc == TSC_EXTERN)
                ssa_prints(self, "external ");

        if (tree_get_decl_storage_duration(decl) == TSD_THREAD)
                ssa_prints(self, "thread_local ");

}

static void ssa_print_function(ssa_printer* self, const ssa_value* val)
{
        self->llvm.tmp_id = 0;
        bool defined = ssa_get_function_blocks_begin(val) != ssa_get_function_blocks_cend(val);

        tree_decl* func = ssa_get_function_entity(val);
        tree_type* func_type = tree_get_decl_type(func);
        ssa_prints(self, defined ? "define " : "declare ");
        ssa_print_linkage(self, func);
        ssa_print_dll_storage_class(self, func);
        ssa_print_cc(self, func_type);
        _ssa_print_type(self, tree_get_func_type_result(func_type), 1);
        ssa_prints(self, " @");
        ssa_print_tree_id(self, tree_get_decl_name(func));
        ssa_print_func_type_params(self, func_type);

        if (!defined)
                return;

        ssa_prints(self, "\n{");
        self->indent_lvl++;
        SSA_FOREACH_FUNCTION_BLOCK(val, block)
        {
                ssa_printc(self, '\n');
                ssa_print_block(self, block);
                ssa_printc(self, '\n');
        }
        self->indent_lvl--;
        ssa_prints(self, "\n}");
}

static void ssa_print_const(ssa_printer* self, ssa_const* cst, bool print_type)
{
        ssa_const_kind kind = ssa_get_const_kind(cst);
        tree_type* t = ssa_get_const_type(cst);
        if (print_type)
        {
                if (kind == SCK_ADDRESS)
                        ssa_print_value_type(self, ssa_get_const_addr(cst));
                else
                        ssa_print_type(self, t);
                ssa_printc(self, ' ');
        }
        switch (kind)
        {
                case SCK_LITERAL:
                        ssa_print_constant_value(self, ssa_get_const_type(cst), ssa_get_const_literal(cst));
                        break;
                case SCK_LIST:
                {
                        struct vec* list = ssa_get_const_list(cst);
                        if (tree_type_is_record(t))
                                ssa_prints(self, "<{ ");
                        else if (tree_type_is_array(t))
                                ssa_prints(self, "[ ");
                        for (int i = 0; i < list->size; i++)
                        {
                                ssa_print_const(self, list->items[i], true);
                                if (i + 1 < list->size)
                                        ssa_prints(self, ", ");
                        }
                        if (tree_type_is_record(t))
                                ssa_prints(self, " }> ");
                        else if (tree_type_is_array(t))
                                ssa_prints(self, " ] ");
                        break;
                }
                case SCK_ADDRESS:
                        ssa_print_value(self, ssa_get_const_addr(cst), false);
                        break;
                case SCK_CAST:
                        ssa_const* operand = ssa_get_const_expr_operand(cst, 0);
                        tree_type* to = ssa_get_const_type(cst);
                        llvm_cast_kind kind = ssa_get_llvm_cast_kind(
                                self->context->target, ssa_get_const_type(operand), to);
                        if (kind == LSK_INVALID)
                                break;
                        ssa_printf(self, "%s (", llvm_cast_table[kind]);
                        ssa_print_const(self, operand, true);
                        ssa_prints(self, " to ");
                        ssa_print_type(self, to);
                        ssa_prints(self, ")");
                        break;
                case SCK_PTRADD:
                {
                        ssa_const* op0 = ssa_get_const_expr_operand(cst, 0);
                        ssa_prints(self, "getelementptr(");
                        ssa_print_type(self, tree_get_pointer_target(ssa_get_const_type(op0)));
                        ssa_prints(self, ", ");
                        ssa_print_const(self, ssa_get_const_expr_operand(cst, 0), true);
                        ssa_prints(self, ", ");
                        ssa_print_const(self, ssa_get_const_expr_operand(cst, 1), true);
                        ssa_printc(self, ')');
                        break;
                }

                case SCK_GETFIELDADDR:
                {
                        ssa_const* var = ssa_get_const_field_addr_var(cst);
                        tree_type* rec_ptr = ssa_get_const_type(var);
                        unsigned index = ssa_get_const_field_addr_index(cst);
                        ssa_prints(self, "getelementptr inbounds(");
                        ssa_print_type(self, tree_get_pointer_target(rec_ptr));
                        ssa_prints(self, ", ");
                        ssa_print_const(self, var, true);
                        ssa_printf(self, ", i32 0, i32 %u", index);
                        ssa_printc(self, ')');
                        break;
                }
                default:
                        break;
        }
}

static void ssa_print_global_var(ssa_printer* self, const ssa_value* val)
{
        tree_decl* decl = ssa_get_global_var_entity(val);
        ssa_print_value(self, val, false);
        ssa_prints(self, " = ");
        ssa_print_linkage(self, decl);
        ssa_prints(self, "global ");
        ssa_print_type(self, tree_get_pointer_target(ssa_get_value_type(val)));
        ssa_const* cst = ssa_get_global_var_init(val);
        if (!cst)
                return;
        ssa_printc(self, ' ');
        ssa_print_const(self, cst, false);  
}

static void ssa_print_global_value(ssa_printer* self, const ssa_value* val)
{
        switch (ssa_get_value_kind(val))
        {
                case SVK_STRING:
                        ssa_print_string_value(self, val);
                        return;
                case SVK_GLOBAL_VAR:
                        ssa_print_global_var(self, val);
                        return;
                case SVK_FUNCTION:
                        ssa_print_function(self, val);
                        return;
                default:
                        return;
        }
}

extern void ssa_pretty_print_module_llvm(
        FILE* fout, ssa_context* context, const ssa_module* module)
{
        ssa_printer p;
        ssa_init_printer(&p, context, fout);
        SSA_FOREACH_MODULE_TYPE_DECL(module, it, end)
                if (tree_decl_is(*it, TDK_RECORD))
                {
                        ssa_print_record_decl(&p, *it);
                        ssa_printc(&p, '\n');
                }
        SSA_FOREACH_MODULE_GLOBAL(module, it, end)
        {
                ssa_print_global_value(&p, *it);
                ssa_printc(&p, '\n');
        }
        ssa_dispose_printer(&p);
}
