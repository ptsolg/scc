#ifndef C_SEMA_H
#define C_SEMA_H

#include "scc/c-common/context.h"
#include "scc/tree/type.h"
#include "scc/tree/expr.h"

typedef struct _tree_stmt tree_stmt;
typedef struct _tree_decl tree_decl;
typedef struct _tree_target_info tree_target_info;
typedef struct _tree_module tree_module;
typedef struct _tree_scope tree_scope;
typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_type tree_type;
typedef struct _tree_expr tree_expr;
typedef struct _c_param c_param;

typedef struct
{
        tree_stmt* switch_stmt;
        struct hashmap labels;
        bool has_default_label;
        bool in_atomic_block;
} c_switch_stmt;

#define VEC   c_switch_stack
#define VEC_T c_switch_stmt
#include "scc/core/vec.inc"

// this is used for semantic analysis and building AST
typedef struct _c_sema
{
        tree_context* context;
        c_context* ccontext;
        tree_decl* function;
        tree_decl_scope* globals;
        tree_decl_scope* labels;
        tree_decl_scope* locals;
        struct c_switch_stack switch_stack;
        tree_scope* scope;
        tree_module* module;
        tree_target_info* target;

        struct
        {
                struct vec non_atomic_gotos;
                struct hashmap atomic_labels;
                int atomic_stmt_nesting;
        } tm_info;
} c_sema;

extern void c_sema_init(c_sema* self, c_context* context);
extern void c_sema_dispose(c_sema* self);

extern tree_module* c_sema_new_module(c_sema* self);
extern void c_sema_enter_scope(c_sema* self, tree_scope* scope);
extern void c_sema_exit_scope(c_sema* self);
extern void c_sema_enter_decl_scope(c_sema* self, tree_decl_scope* scope);
extern void c_sema_exit_decl_scope(c_sema* self);
extern void c_sema_enter_function(c_sema* self, tree_decl* func);
extern void c_sema_exit_function(c_sema* self);

// creates new decl scope, and enters it
// this is used when we need to create implicit decl scope.
// e.g: to hide for-loop variables from using them outside loop scope
extern void c_sema_push_scope(c_sema* self);

extern void c_sema_push_switch_stmt_info(c_sema* self, tree_stmt* switch_stmt);
extern void c_sema_pop_switch_stmt_info(c_sema* self);
extern void c_sema_set_switch_stmt_has_default_label(c_sema* self);
extern void c_sema_set_switch_stmt_in_atomic_block(c_sema* self);
extern c_switch_stmt* c_sema_get_switch_stmt_info(const c_sema* self);
extern bool c_sema_in_switch_stmt(const c_sema* self);
extern bool c_sema_switch_stmt_has_default_label(const c_sema* self);
extern bool c_sema_switch_stmt_in_atomic_block(const c_sema* self);
extern bool c_sema_switch_stmt_register_case_label(const c_sema* self, tree_stmt* label);

extern bool c_sema_at_file_scope(const c_sema* self);
extern bool c_sema_at_block_scope(const c_sema* self);

extern bool c_sema_in_atomic_block(const c_sema* self);
extern bool c_sema_in_transaction_safe_function(const c_sema* self);
extern bool c_sema_in_transaction_safe_block(const c_sema* self);


extern bool c_sema_types_are_same(const c_sema* self, const tree_type* a, const tree_type* b);
extern bool c_sema_types_are_compatible(
        const c_sema* self, const tree_type* a, const tree_type* b, bool unqualify);
extern bool c_sema_require_complete_type(const c_sema* self, tree_location loc, const tree_type* type);

// returns one of: int32_t uint32_t int64_t uint64_t
extern tree_type* c_sema_get_int_type(c_sema* self, bool signed_, bool extended);
extern tree_type* c_sema_get_size_t_type(c_sema* self);
extern tree_type* c_sema_get_float_type(c_sema* self);
extern tree_type* c_sema_get_double_type(c_sema* self);
extern tree_type* c_sema_get_char_type(c_sema* self);
extern tree_type* c_sema_get_logical_operation_type(c_sema* self);
extern tree_type* c_sema_get_type_for_string_literal(c_sema* self, tree_id id);

extern tree_type* c_sema_get_builtin_type(
        c_sema* self, tree_type_quals q, tree_builtin_type_kind k);

extern tree_type* c_sema_new_decl_type(c_sema* self, tree_decl* d, bool referenced);
extern tree_type* c_sema_new_typedef_name(c_sema* self, tree_location name_loc, tree_id name);
extern tree_type* c_sema_new_pointer_type(c_sema* self, tree_type_quals quals, tree_type* target);
extern tree_type* c_sema_new_function_type(c_sema* self, tree_type* restype);
extern tree_type* c_sema_new_paren_type(c_sema* self, tree_type* next);

extern tree_type* c_sema_new_array_type(
        c_sema* self,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size);

extern tree_type* c_sema_new_constant_array_type(
        c_sema* self, tree_type_quals quals, tree_type* eltype, uint size);

extern bool c_sema_typedef_name_exists(c_sema* self, tree_id name);

extern bool c_sema_check_array_type(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_function_type(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_type_quals(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_pointer_type(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_type(const c_sema* self, const tree_type* t, tree_location l);

extern tree_decl* c_sema_local_lookup(const c_sema* self, tree_id name, tree_lookup_kind lk);
extern tree_decl* c_sema_global_lookup(const c_sema* self, tree_id name, tree_lookup_kind lk);
extern tree_decl* c_sema_label_lookup(const c_sema* self, tree_id name);

extern tree_decl* c_sema_require_local_decl(const c_sema* self, tree_location name_loc, tree_id name);
extern tree_decl* c_sema_require_global_decl(const c_sema* self, tree_location name_loc, tree_id name);
extern tree_decl* c_sema_require_label_decl(const c_sema* self, tree_location name_loc, tree_id name);
extern tree_decl* c_sema_require_field_decl(
        const c_sema* self, const tree_decl* rec, tree_location name_loc, tree_id name);

typedef struct
{
        tree_type* head;
        tree_type* tail;
} c_type_chain;

extern void c_type_chain_init(c_type_chain* self);

typedef enum
{
        CDK_UNKNOWN,
        CDK_TYPE_NAME,
        CDK_PARAM,
        CDK_MEMBER,
} c_declarator_kind;

typedef struct _c_declarator
{
        c_declarator_kind kind;
        c_type_chain type;
        tree_id name;
        tree_location name_loc;
        tree_xlocation loc;
        struct vec params;
        bool params_initialized;
        c_context* context;
} c_declarator;

extern void c_declarator_init(c_declarator* self, c_sema* sema, c_declarator_kind k);
extern void c_declarator_dispose(c_declarator* self);
extern void c_declarator_set_name(c_declarator* self, tree_location name_loc, tree_id name);
extern void c_declarator_set_initialized(c_declarator* self);
extern void c_declarator_set_loc_begin(c_declarator* self, tree_location begin);
extern void c_declarator_set_loc_end(c_declarator* self, tree_location end);
extern tree_type* c_declarator_get_type(const c_declarator* self);
extern tree_location c_declarator_get_name_loc_or_begin(const c_declarator* self);

extern bool c_sema_add_direct_declarator_function_suffix(c_sema* self, c_declarator* d);
extern bool c_sema_add_direct_declarator_array_suffix(
        c_sema* self, c_declarator* d, tree_type_quals q, tree_expr* size_expr);
extern void c_sema_add_direct_declarator_transaction_safe_attribute(c_sema* self, c_declarator* d);
extern void c_sema_add_direct_declarator_stdcall_attribute(c_sema* self, c_declarator* d);
extern bool c_sema_add_direct_declarator_parens(c_sema* self, c_declarator* d);
extern void c_sema_add_declarator_param(c_sema* self, c_declarator* d, c_param* p);
extern bool c_sema_set_declarator_has_vararg(c_sema* self, c_declarator* d, tree_location ellipsis_loc);
extern bool c_sema_finish_declarator(c_sema* self, c_declarator* declarator, c_type_chain* pointers);

typedef struct _c_decl_specs
{
        tree_storage_class storage_class;
        tree_storage_duration storage_duration;
        tree_dll_storage_class dll_storage_class;
        tree_type* typespec;
        tree_xlocation loc;
        bool has_inline;
        bool has_typedef;
} c_decl_specs;

extern void c_decl_specs_init(c_decl_specs* self);
extern void c_decl_specs_set_loc_begin(c_decl_specs* self, tree_location begin);
extern void c_decl_specs_set_loc_end(c_decl_specs* self, tree_location end);
extern tree_location c_decl_specs_get_loc_begin(const c_decl_specs* self);
extern tree_location c_decl_specs_get_loc_end(const c_decl_specs* self);

extern bool c_sema_set_type_specifier(c_sema* self, c_decl_specs* ds, tree_type* ts);
extern bool c_sema_set_typedef_specified(c_sema* self, c_decl_specs* ds);
extern bool c_sema_set_inline_specified(c_sema* self, c_decl_specs* ds);
extern bool c_sema_set_storage_class(c_sema* self, c_decl_specs* ds, tree_storage_class sc);
extern bool c_sema_set_thread_storage_duration(c_sema* self, c_decl_specs* ds);
extern bool c_sema_set_dll_storage_class(c_sema* self, c_decl_specs* ds, tree_dll_storage_class sc);
extern tree_decl* c_sema_handle_unused_decl_specs(c_sema* self, c_decl_specs* ds);

typedef struct _c_param
{
        c_decl_specs specs;
        c_declarator declarator;
} c_param;

extern tree_type* c_param_get_type(const c_param* self);
extern tree_xlocation c_param_get_loc(const c_param* self);

extern c_param* c_sema_start_param(c_sema* self);
extern c_param* c_sema_finish_param(c_sema* self, c_param* p);

extern tree_type* c_sema_new_type_name(c_sema* self, c_declarator* d, tree_type* typespec);

extern tree_decl* c_sema_add_decl_to_group(c_sema* self, tree_decl* group, tree_decl* decl);

extern tree_decl* c_sema_define_enumerator_decl(
        c_sema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* value);

extern tree_decl* c_sema_define_enum_decl(c_sema* self, tree_location kw_loc, tree_id name);
extern tree_decl* c_sema_declare_enum_decl(c_sema* self, tree_location kw_loc, tree_id name);
extern tree_decl* c_sema_complete_enum_decl(c_sema* self, tree_decl* enum_, tree_location end);

extern tree_decl* c_sema_define_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union);
extern tree_decl* c_sema_declare_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union);
extern tree_decl* c_sema_complete_record_decl(c_sema* self, tree_decl* rec, tree_location end);

extern tree_decl* c_sema_define_field_decl(c_sema* self, c_decl_specs* ds, c_declarator* d, tree_expr* bit_width);

extern tree_decl* c_sema_define_label_decl(
        c_sema* self, tree_location name_loc, tree_id name, tree_location colon_loc, tree_stmt* stmt);
extern tree_decl* c_sema_declare_label_decl(c_sema* self, tree_location name_loc, tree_id name);

extern tree_decl* c_sema_declare_external_decl(
        c_sema* self, c_decl_specs* ds, c_declarator* d, bool has_init_or_body);
extern bool c_sema_set_var_init(c_sema* self, tree_decl* var, tree_expr* init);
extern void c_sema_set_func_body(c_sema* self, tree_decl* func, tree_stmt* body);

extern tree_expr* c_sema_new_impl_cast(c_sema* self, tree_expr* e, tree_type* t);
extern tree_type* c_sema_lvalue_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_array_to_pointer_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_function_to_pointer_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_integer_promotion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_array_function_to_pointer_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_unary_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_usual_arithmetic_conversion(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, bool convert_lhs);

extern tree_type* c_sema_default_argument_promotion(c_sema* self, tree_expr** e);

typedef enum
{
        CACRK_COMPATIBLE,
        CACRK_INCOMPATIBLE,
        CACRK_RHS_NOT_AN_ARITHMETIC,
        CACRK_RHS_NOT_A_RECORD,
        CACRK_RHS_TRANSACTION_UNSAFE,
        CACRK_INCOMPATIBLE_RECORDS,
        CACRK_QUAL_DISCARTION,
        CACRK_INCOMPATIBLE_POINTERS,
} c_assignment_conversion_result_kind;

typedef struct _c_assignment_conversion_result
{
        c_assignment_conversion_result_kind kind;
        tree_type_quals discarded_quals;
} c_assignment_conversion_result;

extern tree_type* c_sema_assignment_conversion(
        c_sema* self, tree_type* lt, tree_expr** rhs, c_assignment_conversion_result* r);


extern bool c_sema_require_object_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_function_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_integral_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_integer_expr(const c_sema* self, const tree_expr* e);

extern bool c_sema_require_real_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_record_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_array_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_scalar_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_scalar_expr(const c_sema* self, const tree_expr* e);

extern bool c_sema_require_arithmetic_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_real_or_object_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_lvalue_or_function_designator(
        const c_sema* self, const tree_expr* e);

extern bool c_sema_require_modifiable_lvalue(const c_sema* self, const tree_expr* e);

extern bool c_sema_require_compatible_expr_types(
        const c_sema* self, const tree_type* a, const tree_type* b, tree_location l, bool ignore_modifiers);

extern tree_expr* c_sema_new_paren_expr(
        c_sema* self, tree_location lbracket_loc, tree_expr* expr, tree_location rbracket_loc);
extern tree_expr* c_sema_new_decl_expr(c_sema* self, tree_location loc, tree_id name);

extern tree_expr* c_sema_new_sp_floating_literal(c_sema* self, tree_location loc, float v);
extern tree_expr* c_sema_new_dp_floating_literal(c_sema* self, tree_location loc, ldouble v);
extern tree_expr* c_sema_new_character_literal(c_sema* self, tree_location loc, int c);
extern tree_expr* c_sema_new_integer_literal(
        c_sema* self, tree_location loc, uint64_t v, bool signed_, bool ext);
extern tree_expr* c_sema_new_string_literal(c_sema* self, tree_location loc, tree_id ref);

extern tree_expr* c_sema_new_subscript_expr(
        c_sema* self, tree_location loc, tree_expr* lhs, tree_expr* rhs);

extern tree_expr* c_sema_new_call_expr(c_sema* self, tree_location loc, tree_expr* lhs);
extern void c_sema_add_call_expr_arg(c_sema* self, tree_expr* call, tree_expr* arg);
extern tree_expr* c_sema_check_call_expr_args(c_sema* self, tree_expr* call);

extern tree_expr* c_sema_new_member_expr(
        c_sema* self,
        tree_location loc,
        tree_expr* lhs,
        tree_id id,
        tree_location id_loc,
        bool is_arrow);

extern tree_expr* c_sema_new_unary_expr(
        c_sema* self, tree_location loc, tree_unop_kind opcode, tree_expr* expr);

extern tree_expr* c_sema_new_sizeof_expr(
        c_sema* self, tree_location loc, void* operand, bool contains_type);

extern tree_expr* c_sema_new_cast_expr(
        c_sema* self, tree_location loc, tree_type* type, tree_expr* expr);

extern tree_expr* c_sema_new_binary_expr(
        c_sema* self, tree_location loc, tree_binop_kind opcode, tree_expr* lhs, tree_expr* rhs);

extern tree_expr* c_sema_new_conditional_expr(
        c_sema* self,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs);

extern tree_expr* c_sema_finish_expr(c_sema* self, tree_expr* expr);

extern tree_expr* c_sema_new_initializer_list(c_sema* self, tree_location loc);
extern tree_expr* c_sema_add_initializer_list_expr(
        c_sema* self, tree_expr* list, tree_expr* expr);

typedef struct _c_initializer_check_result
{
        tree_expr* syntactical_initializer;
        tree_expr* semantic_initializer;
        bool check_only;
} c_initializer_check_result;

extern tree_expr* c_sema_get_default_initializer(c_sema* self, tree_type* obj, tree_storage_class sc);
extern bool c_sema_check_initializer(
        c_sema* self,
        tree_type* object,
        tree_storage_class object_sc,
        tree_expr* init,
        c_initializer_check_result* result,
        bool check_only);

extern tree_expr* c_sema_new_designation(c_sema* self, tree_location start_loc);
extern tree_expr* c_sema_add_designation_designator(
        c_sema* self, tree_expr* designation, tree_designator* designator);
extern tree_expr* c_sema_set_designation_initializer(c_sema* self, tree_expr* designation, tree_expr* init);

extern tree_designator* c_sema_new_field_designator(c_sema* self, tree_location loc, tree_id field);
extern tree_designator* c_sema_new_array_designator(c_sema* self, tree_location loc, tree_expr* index);

extern void c_sema_add_compound_stmt_item(c_sema* self, tree_stmt* compound, tree_stmt* item);
extern tree_stmt* c_sema_new_compound_stmt(c_sema* self, tree_location lbrace_loc);

extern tree_stmt* c_sema_start_atomic_stmt(c_sema* self, tree_location kw_loc);
extern tree_stmt* c_sema_finish_atomic_stmt(c_sema* self, tree_stmt* atomic, tree_stmt* body);

extern tree_stmt* c_sema_start_case_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_expr* expr);

extern tree_stmt* c_sema_finish_case_stmt(c_sema* self, tree_stmt* stmt, tree_stmt* body);

extern tree_stmt* c_sema_start_default_stmt(
        c_sema* self, tree_location kw_loc, tree_location colon_loc);

extern tree_stmt* c_sema_finish_default_stmt(c_sema* self, tree_stmt* stmt, tree_stmt* body);

extern tree_stmt* c_sema_new_labeled_stmt(c_sema* self, tree_decl* label, tree_stmt* stmt);

extern tree_stmt* c_sema_new_expr_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_expr* expr);

extern tree_stmt* c_sema_new_if_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_);

extern tree_stmt* c_sema_new_decl_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_decl* d);

extern tree_stmt* c_sema_new_switch_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        tree_stmt* body);

extern tree_stmt* c_sema_start_switch_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        int scope_flags);

extern tree_stmt* c_sema_finish_switch_stmt(c_sema* self, tree_stmt* switch_, tree_stmt* body);

extern tree_stmt* c_sema_new_while_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body);

extern tree_stmt* c_sema_new_do_while_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_expr* condition,
        tree_stmt* body);

extern tree_stmt* c_sema_new_for_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_expr* condition,
        tree_expr* step,
        tree_stmt* body);

extern tree_stmt* c_sema_new_goto_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location id_loc,
        tree_id id,
        tree_location semicolon_loc,
        int scope_flags);

extern tree_stmt* c_sema_new_continue_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* c_sema_new_break_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* c_sema_new_return_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc, tree_expr* value);

enum
{
        SCOPE_FLAG_NONE = 0,
        SCOPE_FLAG_BREAK = 1,
        SCOPE_FLAG_CONTINUE = 2,
        SCOPE_FLAG_SWITCH = 4 | SCOPE_FLAG_BREAK,
        SCOPE_FLAG_DECL = 8,
        SCOPE_FLAG_ATOMIC = 16 | SCOPE_FLAG_BREAK,
        SCOPE_FLAG_ITERATION = SCOPE_FLAG_BREAK | SCOPE_FLAG_CONTINUE,
};

extern bool c_sema_check_stmt(const c_sema* self, const tree_stmt* s, int scope_flags);

#endif
