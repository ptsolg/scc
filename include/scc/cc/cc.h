#ifndef CC_H
#define CC_H

#include "scc/scl/file.h"

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
        dseq sources;
        dseq libs;
        file_lookup source_lookup;
        file_lookup lib_lookup;
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

        struct
        {
                bool eliminate_dead_code;
                bool fold_constants;
        } optimization;

        struct
        {
                bool print_expr_value;
                bool print_expr_type;
                bool print_impl_casts;
                bool print_eval_result;
                bool force_brackets;
        } cprint;

        const char* llc_path;
        const char* lld_path;
} cc_opts;

typedef struct _cc_instance
{
        cc_input input;
        cc_output output;
        cc_opts opts;
        allocator* alloc;
} cc_instance;

extern void cc_init(cc_instance* self, FILE* message);
extern void cc_init_ex(cc_instance* self, FILE* message, allocator* alloc);
extern void cc_dispose(cc_instance* self);

extern void cc_set_output_stream(cc_instance* self, FILE* out);
extern serrcode cc_set_output_file(cc_instance* self, const char* file);

extern serrcode cc_add_lib_dir(cc_instance* self, const char* dir);
extern serrcode cc_add_lib(cc_instance* self, const char* lib);
extern serrcode cc_add_source_dir(cc_instance* self, const char* dir);
extern serrcode cc_add_source_file(cc_instance* self, const char* file);
extern serrcode cc_emulate_source_file(
        cc_instance* self, const char* file, const char* content);

extern serrcode cc_parse_opts(cc_instance* self, int argc, const char** argv);
extern serrcode cc_run(cc_instance* self);

#endif