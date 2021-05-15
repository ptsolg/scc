#ifndef CC_H
#define CC_H

#include "scc/core/file.h"
#include "scc/core/vec.h"

typedef enum
{
        CTK_X86_32,
        CTK_X86_64,
} cc_target_kind;

typedef enum
{
        COK_NONE,
        COK_EXEC,
        COK_OBJ,
        COK_LEXEMES,
        COK_C,
        COK_SSA,
        COK_ASM,
        COK_LLVM_IR,
} cc_output_kind;

typedef struct
{
        struct vec sources;
        struct vec builtin_sources;
        struct vec libs;
        struct vec obj_files;
        file_lookup source_lookup;
        file_lookup lib_lookup;
        file_entry* tm_decls;
        const char* llc_path;
        const char* lld_path;
        const char* entry;
} cc_input;

typedef struct
{
        cc_output_kind kind;
        FILE* message;
        FILE* file;
        const char* file_path;
} cc_output;

typedef struct
{
        cc_target_kind target;
        const char* name;

        struct
        {
                bool eliminate_dead_code;
                bool fold_constants;
                bool promote_allocas;
                unsigned level;
        } optimization;

        struct
        {
                bool print_expr_value;
                bool print_expr_type;
                bool print_impl_casts;
                bool print_eval_result;
                bool print_semantic_init;
                bool force_brackets;
        } cprint;

        struct
        {
                bool enable_tm;
        } ext;
} cc_opts;

typedef struct _cc_instance
{
        cc_input input;
        cc_output output;
        cc_opts opts;
} cc_instance;

extern void cc_init(cc_instance* self, FILE* message);
extern void cc_dispose(cc_instance* self);

extern void cc_set_output_stream(cc_instance* self, FILE* out);
extern errcode cc_set_output_file(cc_instance* self, const char* file);

extern void cc_add_lib_dir(cc_instance* self, const char* dir);
extern errcode cc_add_lib(cc_instance* self, const char* lib);
extern void cc_add_source_dir(cc_instance* self, const char* dir);
extern errcode cc_add_source_file(cc_instance* self, const char* file, bool builtin);
extern errcode cc_add_obj_file(cc_instance* self, const char* file);
extern errcode cc_emulate_source_file(
        cc_instance* self, const char* file, const char* content, bool builtin, bool add_to_input);

extern errcode cc_run(cc_instance* self);

#endif
