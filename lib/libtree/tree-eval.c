#include "tree-eval.h"
#include "tree-exp.h"

static bool tree_eval_sizeof(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_paren(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_cast(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_numeric_literal(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_conditional(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_invalid_unop(const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_plus(const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_minus(const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_not(const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_log_not(const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool(*const tree_unop_eval_table[])(
        const tree_target_info*,
        tree_eval_result*) =
{
        &tree_eval_invalid_unop, // TUK_UNKNOWN
        &tree_eval_invalid_unop, // TUK_POST_INC
        &tree_eval_invalid_unop, // TUK_POST_DEC
        &tree_eval_invalid_unop, // TUK_PRE_INC
        &tree_eval_invalid_unop, // TUK_PRE_DEC
        &tree_eval_plus,         // TUK_PLUS
        &tree_eval_minus,        // TUK_MINUS
        &tree_eval_not,          // TUK_NOT
        &tree_eval_log_not,      // TUK_LOG_NOT
        &tree_eval_invalid_unop, // TUK_DEREFERENCE
        &tree_eval_invalid_unop, // TUK_ADDRESS
};

static bool tree_eval_unop(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        const tree_type* t = tree_desugar_ctype(tree_get_exp_type(e));
        if (tree_get_type_kind(t) != TTK_BUILTIN)
                return false;

        return tree_unop_eval_table[tree_get_unop_kind(e)](i, r);
}

typedef struct
{
        const tree_target_info* platform;
        tree_eval_result          lhs;
        tree_eval_result          rhs;
} tree_binop_eval_info;

static bool tree_eval_invalid_binop(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_mul(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_div(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_mod(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_add(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_sub(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_shl(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_shr(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_le(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_gr(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_leq(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_geq(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_eq(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_neq(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_and(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_xor(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_or(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_log_and(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool tree_eval_log_or(const tree_binop_eval_info* i, tree_eval_result* r)
{
        return false;
}

static bool(*const tree_binop_eval_table[])(const tree_binop_eval_info*, tree_eval_result*) =
{
        &tree_eval_invalid_binop, // TBK_UNKNOWN
        &tree_eval_mul,           // TBK_MUL
        &tree_eval_div,           // TBK_DIV
        &tree_eval_mod,           // TBK_MOD
        &tree_eval_add,           // TBK_ADD
        &tree_eval_sub,           // TBK_SUB
        &tree_eval_shl,           // TBK_SHL
        &tree_eval_shr,           // TBK_SHR
        &tree_eval_le,            // TBK_LE
        &tree_eval_gr,            // TBK_GR
        &tree_eval_leq,           // TBK_LEQ
        &tree_eval_geq,           // TBK_GEQ
        &tree_eval_eq,            // TBK_EQ
        &tree_eval_neq,           // TBK_NEQ
        &tree_eval_and,           // TBK_AND
        &tree_eval_xor,           // TBK_XOR
        &tree_eval_or,            // TBK_OR
        &tree_eval_log_and,       // TBK_LOG_AND
        &tree_eval_log_or,        // TBK_LOG_OR
        &tree_eval_invalid_binop, // TBK_ASSIGN
        &tree_eval_invalid_binop, // TBK_ADD_ASSIGN
        &tree_eval_invalid_binop, // TBK_SUB_ASSIGN
        &tree_eval_invalid_binop, // TBK_MUL_ASSIGN
        &tree_eval_invalid_binop, // TBK_DIV_ASSIGN
        &tree_eval_invalid_binop, // TBK_MOD_ASSIGN
        &tree_eval_invalid_binop, // TBK_SHL_ASSIGN
        &tree_eval_invalid_binop, // TBK_SHR_ASSIGN
        &tree_eval_invalid_binop, // TBK_AND_ASSIGN
        &tree_eval_invalid_binop, // TBK_XOR_ASSIGN
        &tree_eval_invalid_binop, // TBK_OR_ASSIGN
        &tree_eval_invalid_binop, // TBK_COMMA
};

static bool tree_eval_binop(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        const tree_exp* lhs = tree_get_binop_lhs(e);
        const tree_exp* rhs = tree_get_binop_rhs(e);
        const tree_type* lt = tree_desugar_ctype(tree_get_exp_type(lhs));
        const tree_type* rt = tree_desugar_ctype(tree_get_exp_type(rhs));
        if (tree_get_type_kind(lt) != TTK_BUILTIN || tree_get_type_kind(rt) != TTK_BUILTIN)
                return false;
        if (tree_get_builtin_type_kind(lt) != tree_get_builtin_type_kind(rt))
                return false;

        tree_binop_eval_info info = { .platform = i };
        if (!tree_eval_exp(lhs, i, &info.lhs) || !tree_eval_exp(rhs, i, &info.rhs))
                return false;

        return tree_binop_eval_table[tree_get_binop_kind(e)](&info, r);
}

static bool tree_eval_invalid(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return false;
}

static bool(*const tree_eval_table[])(
        const tree_exp*,
        const tree_target_info*,
        tree_eval_result*) =
{
        &tree_eval_invalid,         // TEK_UNKNOWN
        &tree_eval_binop,           // TEK_BINARY
        &tree_eval_unop,            // TEK_UNARY
        &tree_eval_invalid,         // TEK_CALL
        &tree_eval_invalid,         // TEK_SUBSCRIPT
        &tree_eval_conditional,     // TEK_CONDITIONAL
        &tree_eval_numeric_literal, // TEK_INTEGER_LITERAL
        &tree_eval_numeric_literal, // TEK_CHARACTER_LITERAL
        &tree_eval_numeric_literal, // TEK_FLOATING_LITERAL
        &tree_eval_invalid,         // TEK_STRING_LITERAL
        &tree_eval_invalid,         // TEK_DECL
        &tree_eval_invalid,         // TEK_MEMBER
        &tree_eval_cast,            // TEK_EXPLICIT_CAST
        &tree_eval_cast,            // TEK_IMPLICIT_CAST
        &tree_eval_sizeof,          // TEK_SIZEOF
        &tree_eval_paren,           // TEK_PAREN
        &tree_eval_invalid,         // TEK_INIT
};                         
                           
extern bool tree_eval_exp(const tree_exp* e, const tree_target_info* i, tree_eval_result* r)
{
        return tree_eval_table[tree_get_exp_kind(e)](e, i, r);
}