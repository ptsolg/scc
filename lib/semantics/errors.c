#include "errors.h"
#include "scc/c-common/context.h"
#include "scc/semantics/sema.h"
#include "scc/tree/context.h"
#include "scc/tree/stmt.h"

static const char* c_context_get_id_string(c_context* self, tree_id id)
{
        return tree_get_id_string(self->tree, id);
}

extern void c_error_too_many_initializer_values(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "too many initializer values");
}

extern void c_error_undeclared_identifier(c_context* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "undeclared identifier '%s'", c_context_get_id_string(self, name));
}

extern void c_error_multiple_storage_classes(c_context* self, const c_decl_specs* ds)
{
        c_error(self, CES_ERROR, ds->loc.begin,
                "multiple storage classes in declaration specifiers");
}

extern void c_error_invalid_specifier(c_context* self, tree_location loc, const char* spec, int allowed_decl)
{
        const char* decl = "<unknown decl>";
        if (allowed_decl == TDK_VAR)
                decl = "variable";
        else if (allowed_decl == TDK_FUNCTION)
                decl = "function";

        c_error(self, CES_ERROR, loc, "'%s' specifier allowed on %s declarations only", spec, decl);
}

extern void c_error_invalid_parameter_storage_class(c_context* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, d->name_loc, "invalid storage class for parameter '%s'",
                c_context_get_id_string(self, d->name));
}

extern void c_error_variable_declared_thread_local_at_function_scope(c_context* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "variable '%s' declared '_Thread_local' at function scope",
                c_context_get_id_string(self, d->name));
}

extern void c_error_variable_declared_register_at_file_scope(c_context* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "variable '%s' declared 'register' at file scope",
                c_context_get_id_string(self, d->name));
}

extern void c_error_field_declared_with_storage_specifier(c_context* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "field '%s' declared with storage specifier", c_context_get_id_string(self, d->name));
}

extern void c_error_dllimport_cannot_be_applied_to_definition(c_context* self, const c_decl_specs* specs, int decl_kind)
{
        assert(decl_kind == TDK_FUNCTION || decl_kind == TDK_VAR);
        const char* decl = decl_kind == TDK_FUNCTION ? "function" : "variable";
        c_error(self, CES_ERROR, specs->loc.begin,
                "'_Dllimport' specifier cannot be applied to %s definition", decl);
}

extern void c_error_dllimport_applied_to_wrong_decl(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc,
                "'_Dllimport' specifier allowed on function or variable declarations only");
}

extern void c_error_extern_variable_has_an_initializer(c_context* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "'extern' variable has an initializer");
}

extern void c_error_function_initialized_like_a_variable(c_context* self, const tree_decl* func)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(func),
                "function '%s' is initialized like a variable",
                c_context_get_id_string(self, tree_get_decl_name(func)));
}

extern void c_error_invalid_initializer(c_context* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(init), "invalid initializer");
}

static void c_get_qual_string(tree_type_quals q, char* buf)
{
        *buf = '\0';
        if (q & TTQ_CONST)
                strcat(buf, "const ");
        if (q & TTQ_VOLATILE)
                strcat(buf, "volatile ");
        if (q & TTQ_RESTRICT)
                strcat(buf, "restrict ");

        if (buf[0])
                buf[strlen(buf) - 1] = '\0';
}


extern void c_error_initialization_discards_qualifer(c_context* self, const tree_expr* init, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, tree_get_expr_loc(init),
                "initialization discards '%s' qualifier", qstring);
}

extern void c_error_initialization_from_incompatible_pointer_types(c_context* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(init),
                "initialization from incompatible pointer types");
}

extern void c_error_braces_around_scalar_initializer(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "braces around scalar initializer");
}

extern void c_error_field_name_not_in_record_initializer(c_context* self, const tree_designator* d)
{
        c_error(self, CES_ERROR, tree_get_designator_loc(d),
                "field name not in record or union initializer");
}

extern void c_error_array_index_in_non_array_intializer(c_context* self, const tree_designator* d)
{
        c_error(self, CES_ERROR, tree_get_designator_loc(d), "array index in non-array initializer");
}

extern void c_error_array_index_in_initializer_not_of_integer_type(c_context* self, const tree_expr* index)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(index),
                "array index in initializer not of integer type");
}

extern void c_error_nonconstant_array_index_in_initializer(c_context* self, const tree_expr* index)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(index), "nonconstant array index in initializer");
}

extern void c_error_array_index_in_initializer_exceeds_array_bounds(c_context* self, const tree_expr* index)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(index),
                "array index in initializer exceeds array bounds");
}

extern void c_error_initializer_element_isnt_constant(c_context* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc_begin(init), "initializer element is not constant");
}

extern void c_error_initializer_string_is_too_long(c_context* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc_begin(init), "initializer-string is too long");
}

extern void c_error_array_cannot_be_initalized_with_string_literal(c_context* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc_begin(init),
                "array of inappropriate type initialized from string constant");
}

extern void c_error_named_argument_before_ellipsis_required(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "ISO C requires a named argument before '...'");
}

extern void c_error_redefinition(c_context* self, tree_location loc, tree_id id)
{
        c_error(self, CES_ERROR, loc, "redefinition of '%s'", c_context_get_id_string(self, id));
}

extern void c_error_decl_redefinition(c_context* self, const tree_decl* decl)
{
        c_error_redefinition(self, tree_get_decl_loc_begin(decl), tree_get_decl_name(decl));
}

extern void c_error_enumerator_value_isnt_constant(c_context* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "enumerator value for '%s' is not an integer constant",
                c_context_get_id_string(self, name));
}

extern void c_error_wrong_kind_of_tag(c_context* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "'%s' defined as wrong kind of tag",
                c_context_get_id_string(self, name));
}

static void c_error_decl(c_context* self, const char* msg, const tree_decl* decl)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(decl), msg,
                c_context_get_id_string(self, tree_get_decl_name(decl)));
}

extern void c_error_field_function(c_context* self, const tree_decl* field)
{
        c_error_decl(self, "field '%s' declared as function", field);
}

extern void c_error_invalid_bitfield_type(c_context* self, const tree_decl* field)
{
        c_error_decl(self, "bit-field '%s' has invalid type", field);
}

extern void c_error_bitfield_width_isnt_constant(c_context* self, const tree_decl* field)
{
        c_error_decl(self, "bit-field '%s' width not an integer constant", field);
}

extern void c_error_bitfield_width_is_zero(c_context* self, const tree_decl* field)
{
        c_error_decl(self, "zero width for bit-field '%s'", field);
}

extern void c_error_negative_bitfield_width(c_context* self, const tree_decl* field)
{
        c_error_decl(self, "negative width in bit-field '%s'", field);
}

extern void c_error_bitfield_width_exceeds_type(c_context* self, const tree_decl* field)
{
        c_error_decl(self, "width of '%s' exceeds its type", field);
}

extern void c_error_invalid_storage_class(c_context* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "invalid storage class for '%s'", c_context_get_id_string(self, name));
}

extern void c_error_different_kind_of_symbol(c_context* self, const tree_decl* decl)
{
        c_error_decl(self, "'%s' redeclared as different kind of symbol", decl);
}

extern void c_error_different_storage_class(c_context* self, const tree_decl* decl)
{
        c_error_decl(self, "redefinition of '%s' with different storage class", decl);
}

extern void c_error_conflicting_types(c_context* self, const tree_decl* decl)
{
        c_error_decl(self, "conflicting types for '%s'", decl);
}

extern void c_error_parameter_name_omitted(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "parameter name omitted");
}

extern void c_error_function_isnt_allowed_here(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "function definition is not allowed here");
}

extern void c_error_expr_must_have_pointer_to_object_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have pointer-to-object type");
}

extern void c_error_expr_must_have_pointer_to_function_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have pointer-to-function type");
}

extern void c_error_expr_must_have_integral_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have integral type");
}

extern void c_error_expr_must_have_real_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have real type");
}

extern void c_error_expr_must_have_record_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have struct or union type");
}

extern void c_error_expr_must_have_array_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have array type");
}

extern void c_error_expr_must_have_scalar_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have scalar type");
}

extern void c_error_expr_must_have_arithmetic_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have arithmetic type");
}

extern void c_error_expr_must_have_real_or_pointer_to_object_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have real or pointer-to-object type");
}

extern void c_error_expr_must_be_lvalue_or_function_designator(c_context* self, const tree_expr* e)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(e),
                "expression must be an lvalue or function designator");
}

extern void c_error_expr_must_be_modifiable_lvalue(c_context* self, const tree_expr* e)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(e), "expression must be a modifiable lvalue");
}

extern void c_error_types_are_not_compatible(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "types are not compatible");
}

extern void c_error_subscripted_value_isnt_array(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "subscripted value is not array, pointer, or vector");
}

extern void c_error_incompatible_type_for_argument(c_context* self, tree_location loc, unsigned pos)
{
        c_error(self, CES_ERROR, loc,
                "incompatible type for argument %u in function call", pos);
}

extern void c_error_passing_argument_discards_qualifer(
        c_context* self, tree_location loc, unsigned pos, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, loc,
                "passing argument %u discards '%s' qualifier", pos, qstring);
}

extern void c_error_passing_argument_from_incompatible_pointer_type(
        c_context* self, tree_location loc, unsigned pos)
{
        c_error(self, CES_ERROR, loc,
                "passing argument %u from incompatible pointer type", pos);
}

extern void c_error_invalid_scalar_initialization(c_context* self, const tree_expr* e, const c_assignment_conversion_result* r)
{
        switch (r->kind)
        {
        case CACRK_INCOMPATIBLE:
        case CACRK_RHS_NOT_A_RECORD:
        case CACRK_INCOMPATIBLE_RECORDS:
                c_error_invalid_initializer(self, e);
                return;
        case CACRK_RHS_NOT_AN_ARITHMETIC:
                c_error_expr_must_have_arithmetic_type(self, tree_get_expr_loc(e));
                return;
        case CACRK_QUAL_DISCARTION:
                c_error_initialization_discards_qualifer(self, e, r->discarded_quals);
                return;
        case CACRK_INCOMPATIBLE_POINTERS:
        case CACRK_RHS_TRANSACTION_UNSAFE:
                c_error_initialization_from_incompatible_pointer_types(self, e);
                return;
        default:
                UNREACHABLE();
        case CACRK_COMPATIBLE:
                return;
        }
}

extern void c_error_invalid_argument_assignment(c_context* self, tree_location loc, unsigned pos, const c_assignment_conversion_result* r)
{
        switch (r->kind)
        {
        case CACRK_RHS_NOT_AN_ARITHMETIC:
        case CACRK_RHS_NOT_A_RECORD:
        case CACRK_INCOMPATIBLE_RECORDS:
        case CACRK_INCOMPATIBLE:
                c_error_incompatible_type_for_argument(self, loc, pos);
                return;
        case CACRK_QUAL_DISCARTION:
                c_error_passing_argument_discards_qualifer(self, loc, pos, r->discarded_quals);
                return;
        case CACRK_INCOMPATIBLE_POINTERS:
        case CACRK_RHS_TRANSACTION_UNSAFE:
                c_error_passing_argument_from_incompatible_pointer_type(self, loc, pos);
                return;
        default:
                UNREACHABLE();
        case CACRK_COMPATIBLE:
                return;
        }
}

extern void c_error_invalid_assignment(c_context* self, tree_location op_loc,
        const tree_expr* rhs, const c_assignment_conversion_result* r)
{
        tree_location rloc = tree_get_expr_loc(rhs);
        switch (r->kind)
        {
        case CACRK_RHS_NOT_AN_ARITHMETIC:
                c_error_expr_must_have_arithmetic_type(self, rloc);
                return;
        case CACRK_RHS_NOT_A_RECORD:
                c_error_expr_must_have_record_type(self, rloc);
                return;
        case CACRK_INCOMPATIBLE_RECORDS:
                c_error_types_are_not_compatible(self, rloc);
                return;
        case CACRK_INCOMPATIBLE:
                c_error_invalid_binop_operands(self, op_loc, TBK_ASSIGN);
                return;
        case CACRK_QUAL_DISCARTION:
                c_error_assignment_discards_quals(self, op_loc, r->discarded_quals);
                return;
        case CACRK_INCOMPATIBLE_POINTERS:
        case CACRK_RHS_TRANSACTION_UNSAFE:
                c_error_assignment_from_incompatible_pointer_type(self, op_loc);
                return;
        default:
                UNREACHABLE();
        case CACRK_COMPATIBLE:
                return;
        }
}

extern void c_error_invalid_return_type(c_context* self, const tree_expr* e, const c_assignment_conversion_result* r)
{
        tree_location loc = tree_get_expr_loc(e);
        switch (r->kind)
        {
        case CACRK_RHS_NOT_AN_ARITHMETIC:
                c_error_expr_must_have_arithmetic_type(self, loc);
                return;
        case CACRK_RHS_NOT_A_RECORD:
                c_error_expr_must_have_record_type(self, loc);
                return;
        case CACRK_INCOMPATIBLE_RECORDS:
                c_error_types_are_not_compatible(self, loc);
                return;
        case CACRK_INCOMPATIBLE:
                c_error_return_type_doesnt_match(self, e);
                return;
        case CACRK_QUAL_DISCARTION:
                c_error_return_discards_quals(self, loc, r->discarded_quals);
                return;
        case CACRK_INCOMPATIBLE_POINTERS:
        case CACRK_RHS_TRANSACTION_UNSAFE:
                c_error_return_from_incompatible_pointer_type(self, loc);
                return;

        default:
                UNREACHABLE();
        case CACRK_COMPATIBLE:
                return;
        }
}

extern void c_error_too_many_arguments(c_context* self, const tree_expr* call)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(call),
                "too many arguments in function call");
}

extern void c_error_too_few_arguments(c_context* self, const tree_expr* call)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(call),
                "too few arguments in function call");
}

extern void c_error_operand_of_sizeof_is_function(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "operand of sizeof may not be a function");
}

extern void c_error_operand_of_sizeof_is_bitfield(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "operand of sizeof may not be a bitfield");
}

static const char* binop2str[] = {
        "",    // TBK_UNKNOWN
        "*",   // TBK_MUL
        "/",   // TBK_DIV
        "%",   // TBK_MOD
        "+",   // TBK_ADD
        "-",   // TBK_SUB
        "<<",  // TBK_SHL
        ">>",  // TBK_SHR
        "<",   // TBK_LE
        ">",   // TBK_GR
        "<=",  // TBK_LEQ
        ">=",  // TBK_GEQ
        "==",  // TBK_EQ
        "!=",  // TBK_NEQ
        "&",   // TBK_AND
        "^",   // TBK_XOR
        "|",   // TBK_OR
        "&&",  // TBK_LOG_AND
        "||",  // TBK_LOG_OR
        "=",   // TBK_ASSIGN
        "+=",  // TBK_ADD_ASSIGN
        "-=",  // TBK_SUB_ASSIGN
        "*=",  // TBK_MUL_ASSIGN
        "/=",  // TBK_DIV_ASSIGN
        "%=",  // TBK_MOD_ASSIGN
        "<<=", // TBK_SHL_ASSIGN
        ">>=", // TBK_SHR_ASSIGN
        "&=",  // TBK_AND_ASSIGN
        "^=",  // TBK_XOR_ASSIGN
        "|=",  // TBK_OR_ASSIGN
        ",",   // TBK_COMMA
};

extern void c_error_invalid_binop_operands(c_context* self, tree_location loc, int opcode)
{
        c_error(self, CES_ERROR, loc,
                "invalid operands to binary '%s'", binop2str[opcode]);
}

extern void c_error_cmp_of_distinct_pointers(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "comparison of distinct pointer types");
}

extern void c_error_assignment_discards_quals(c_context* self, tree_location loc, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, loc, "assignment discards '%s' qualifier", qstring);
}

extern void c_error_assignment_from_incompatible_pointer_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "assignment from incompatible pointer type");
}

extern void c_error_pointer_type_mismatch(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "pointer type mismatch in conditional expression");
}

extern void c_error_type_mismatch(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "type mismatch in conditional expression");
}

extern void c_error_case_stmt_outside_switch(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "a case statement may only be used within a switch");
}

extern void c_error_case_stmt_isnt_constant(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "case label does not reduce to an integer constant");
}

extern void c_error_case_stmt_duplication(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "duplicate case value");
}

extern void c_error_default_stmt_outside_switch(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "a default statement may only be used within a switch");
}

extern void c_error_default_stmt_duplication(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "multiple default labels in one switch");
}

extern void c_error_non_variable_decl_in_for_loop(c_context* self, const tree_decl* decl)
{
        c_error_decl(self, "declaration of non-variable '%s' in 'for' loop", decl);
}

static const char* c_get_decl_storage_class_string(tree_storage_class sc)
{
        if (sc == TSC_NONE || sc == TSC_IMPL_EXTERN)
                return "";
        else if (sc == TSC_EXTERN)
                return "extern";
        else if (sc == TSC_REGISTER)
                return "register";
        else if (sc == TSC_STATIC)
                return "static";
        return "";
}

extern void c_error_invalid_storage_class_for_loop_decl(c_context* self, const tree_decl* decl)
{
        tree_storage_class sc = tree_get_decl_storage_class(decl);
        c_error(self, CES_ERROR,
                tree_get_decl_loc_begin(decl),
                "declaration of '%s' variable '%s' in 'for' loop initial declaration",
                c_get_decl_storage_class_string(sc),
                c_context_get_id_string(self, tree_get_decl_name(decl)));
}

extern void c_error_return_non_void(c_context* self, const tree_expr* expr)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(expr),
                "return with a value, in function returning void");
}

extern void c_error_return_type_doesnt_match(c_context* self, const tree_expr* expr)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(expr),
                "return type does not match the function type");
}

extern void c_error_return_discards_quals(c_context* self, tree_location loc, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, loc, "return discards '%s' qualifier", qstring);
}

extern void c_error_return_from_incompatible_pointer_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "return from incompatible pointer type");
}

static void c_error_stmt(c_context* self, const char* msg, const tree_stmt* stmt)
{
        c_error(self, CES_ERROR, tree_get_stmt_loc(stmt).begin, msg);
}

extern void c_error_break_stmt_outside_loop_or_switch(c_context* self, const tree_stmt* stmt)
{
        c_error_stmt(self,
                "a break statement may only be used within a loop or switch", stmt);
}

extern void c_error_continue_stmt_outside_loop(c_context* self, const tree_stmt* stmt)
{
        c_error_stmt(self,
                "a continue statement may only be used within a loop", stmt);
}

extern void c_error_decl_stmt_outside_block(c_context* self, const tree_stmt* stmt)
{
        c_error_stmt(self,
                "a declaration may only be used within a block", stmt);
}

extern void c_error_undefined_label(c_context* self, const tree_decl* label)
{
        c_error_decl(self, "label '%s' used but not defined", label);
}

extern void c_error_incomplete_type(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "incomplete type is not allowed");
}

extern void c_error_array_of_functions(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "array of functions is not allowed");
}

extern void c_error_array_size_isnt_integer(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "size of array has non-integer type");
}

extern void c_error_array_must_be_greater_than_zero(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "size of array must be greater than zero");
}

extern void c_error_function_returning_array(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "function returning array is not allowed");
}

extern void c_error_function_returning_function(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "function returning function is not allowed");
}

extern void c_error_invalid_use_of_restrict(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "invalid use of 'restrict'");
}

extern void c_error_reffering_volatile_object_is_not_allowed(c_context* self, tree_location loc, bool in_atomic_block)
{
        const char* context = in_atomic_block ? "_Atomic block" : "_Transaction_safe function";
        c_error(self, CES_ERROR, loc, "volatile objects cannot be used within %s", context);
}

extern void c_error_transaction_unsafe_function_is_not_allowed(c_context* self, tree_location loc, bool in_atomic_block)
{
        const char* context = in_atomic_block ? "_Atomic block" : "_Transaction_safe function";
        c_error(self, CES_ERROR, loc, "calling transaction-unsafe functions is not allowed within %s", context);
}

extern void c_error_volatile_param_is_not_allowed(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "volatile parameter cannot appear in _Transaction_safe function");
}

extern void c_error_jumping_into_the_atomic_block_is_prohibited(c_context* self, tree_location loc, const char* stmt)
{
        c_error(self, CES_ERROR, loc,
                "jumping into the body of _Atomic block using %s statement is prohibited", stmt);
}
