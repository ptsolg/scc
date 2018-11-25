// wraps expressions in parens

// todo: add parens to types, typedefs

#include <stdio.h>
#include "scc/c/c.h"
#include "scc/c/printer.h"
#include "scc/c/context.h"
#include "scc/tree/context.h"
#include "scc/tree/target.h"
#include "scc/tree/stmt.h"
#include "scc/tree/module.h"
#include "scc/tree/expr.h"

char cwd[MAX_PATH_LEN + 1];
jmp_buf on_err;
file_lookup fl;
tree_target_info target;
tree_context tctx;
c_context cctx;
c_printer printer;
fwrite_cb cb;
FILE* fout;

static char* init_all(int argc, const char** argv, jmp_buf on_err)
{
        flookup_init(&fl);
        tree_init_target_info(&target, TTAK_X86_32);
        tree_init(&tctx, &target);
        c_context_init(&cctx, &tctx, &fl, NULL, on_err);

        if (EC_FAILED(flookup_add(&fl, cwd)))
                return "Internal error.";
        if (!(fout = fopen(argv[2], "w")))
                return "Cannot open file.";

        fwrite_cb_init(&cb, fout);
        c_printer_init(&printer, &cb.base, &cctx);
        return NULL;
}

static void dispose_all()
{
        c_context_dispose(&cctx);
        tree_dispose(&tctx);
        flookup_dispose(&fl);
        c_printer_dispose(&printer);
        fclose(fout);
}

static tree_module* parse_file(const char* file)
{
        file_entry* fe = file_get(&fl, file);
        return fe ? c_parse_source(&cctx, fe, stderr) : NULL;
}

static void wrap_decl(tree_decl* d);
static tree_expr* wrap_expr(tree_expr* e);
static void wrap_stmt(tree_stmt* s);

static tree_expr* add_parens(tree_expr* e)
{
        tree_value_kind vk = tree_get_expr_value_kind(e);
        tree_type* t = tree_get_expr_type(e);
        return tree_new_paren_expr(&tctx, vk, t, TREE_INVALID_LOC, e);
}

static tree_expr* wrap_expr(tree_expr* e)
{
        if (!e)
                return NULL;

        switch (tree_get_expr_kind(e))
        {
                case TEK_INTEGER_LITERAL:
                case TEK_CHARACTER_LITERAL:
                case TEK_FLOATING_LITERAL:
                case TEK_STRING_LITERAL:
                case TEK_DECL:
                        return add_parens(e);
                case TEK_MEMBER:
                        tree_set_member_expr_lhs(e, wrap_expr(tree_get_member_expr_lhs(e)));
                        return add_parens(e);
                case TEK_CALL:
                        tree_set_call_lhs(e, wrap_expr(tree_get_call_lhs(e)));
                        TREE_FOREACH_CALL_ARG(e, it)
                                *it = wrap_expr(*it);
                        return add_parens(e);
                case TEK_CONDITIONAL:
                        tree_set_conditional_lhs(e, wrap_expr(tree_get_conditional_lhs(e)));
                        tree_set_conditional_rhs(e, wrap_expr(tree_get_conditional_rhs(e)));
                        tree_set_conditional_condition(e, wrap_expr(tree_get_conditional_condition(e)));
                        return add_parens(e);
                case TEK_SIZEOF:
                        return add_parens(e);
                case TEK_BINARY:
                        tree_set_binop_lhs(e, wrap_expr(tree_get_binop_lhs(e)));
                        tree_set_binop_rhs(e, wrap_expr(tree_get_binop_rhs(e)));
                        return add_parens(e);
                case TEK_UNARY:
                        tree_set_unop_operand(e, wrap_expr(tree_get_unop_operand(e)));
                        return add_parens(e);
                case TEK_SUBSCRIPT:
                        tree_set_subscript_lhs(e, wrap_expr(tree_get_subscript_lhs(e)));
                        tree_set_subscript_rhs(e, wrap_expr(tree_get_subscript_rhs(e)));
                        return add_parens(e);
                default:
                        return e;
        }
}

static void wrap_stmt(tree_stmt* s)
{
        if (!s)
                return;
        switch (tree_get_stmt_kind(s))
        {
                case TSK_CASE:
                        tree_set_case_expr(s, wrap_expr(tree_get_case_expr(s)));
                        wrap_stmt(tree_get_case_body(s));
                        return;
                case TSK_EXPR:
                        tree_set_expr_stmt_root(s, wrap_expr(tree_get_expr_stmt_root(s)));
                        return;
                case TSK_IF:
                        tree_set_if_condition(s, wrap_expr(tree_get_if_condition(s)));
                        wrap_stmt(tree_get_if_body(s));
                        wrap_stmt(tree_get_if_else(s));
                        return;
                case TSK_SWITCH:
                        tree_set_switch_expr(s, wrap_expr(tree_get_switch_expr(s)));
                        wrap_stmt(tree_get_switch_body(s));
                        return;
                case TSK_WHILE:
                        tree_set_while_condition(s, wrap_expr(tree_get_while_condition(s)));
                        wrap_stmt(tree_get_while_body(s));
                        return;
                case TSK_DO_WHILE:
                        tree_set_do_while_condition(s, wrap_expr(tree_get_do_while_condition(s)));
                        wrap_stmt(tree_get_do_while_body(s));
                        return;
                case TSK_FOR:
                        wrap_stmt(tree_get_for_init(s));
                        tree_set_for_condition(s, wrap_expr(tree_get_for_condition(s)));
                        tree_set_for_step(s, wrap_expr(tree_get_for_step(s)));
                        wrap_stmt(tree_get_for_body(s));
                        return;
                case TSK_DECL:
                        wrap_decl(tree_get_decl_stmt_entity(s));
                        return;
                case TSK_RETURN:
                        tree_set_return_value(s, wrap_expr(tree_get_return_value(s)));
                        return;
                case TSK_COMPOUND:
                        TREE_FOREACH_STMT(tree_get_compound_scope(s), it)
                                wrap_stmt(it);
                        return;
                default:
                        return;
        }
}

static void wrap_decl(tree_decl* d)
{
        if (tree_decl_is(d, TDK_VAR))
        {
                tree_expr* init = tree_get_var_init(d);
                if (init)
                        tree_set_var_init(d, wrap_expr(init));
        }
        else if (tree_decl_is(d, TDK_FUNCTION))
                wrap_stmt(tree_get_func_body(d));
}

static const char* _main(int argc, const char** argv)
{
        char* err = init_all(argc, argv, on_err);
        if (err)
                return err;

        if (setjmp(on_err))
                return "Fatal error.";

        tree_module* m = parse_file(argv[1]);
        if (!m)
                return "Cannot parse file.";

        TREE_FOREACH_DECL_IN_SCOPE(tree_get_module_globals(m), it)
                wrap_decl(it);

        c_print_module(&printer, m);
        return NULL;
}

int main(int argc, const char** argv)
{
        if (argc != 3)
        {
                printf("Invalid amount of arguments.");
                return 0;
        }
        if (EC_FAILED(path_get_cd(cwd)))
        {
                printf("Internal error.");
                return 0;
        }

        const char* err = _main(argc, argv);
        if (err)
                printf("%s\n", err);
        dispose_all();
        return 0;
}