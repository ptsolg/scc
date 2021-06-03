#ifndef CC_LLVM_H
#define CC_LLVM_H

#include "scc/core/common.h"
#include "scc/core/vec.h"
#include "scc/core/file.h"

enum
{
        LLC_O0,
        LLC_O1,
        LLC_O2,
        LLC_O3,
        LLC_OBJ,
        LLC_ASM,
        LLC_X86,
        LLC_X64,
};

#define LLC_NATIVE_NAME "llc.exe"
#define LLC_MAX_OPTS 64

struct llc
{
        struct pathbuf path;
        char opts[LLC_MAX_OPTS];
        int num_opts;
        const char* input;
        const char* output;
        int is_clang;
};

void llc_init(struct llc* self, const char* llc_path);
bool llc_try_detect(struct llc* self);
void llc_add_opt(struct llc* self, int opt);
void llc_set_input(struct llc* self, const char* in);
void llc_set_output(struct llc* self, const char* out);
int llc_run(struct llc* self);

enum
{
        LLD_DEBUG_FULL,
};

#define LLD_NATIVE_NAME "lld-link.exe"
#define LLD_MAX_OPTS 64

struct lld
{
        struct pathbuf path;
        struct vec files;
        struct vec dirs;
        const char* output;
        const char* entry;
        char opts[LLD_MAX_OPTS];
        int num_opts;
};

void lld_init(struct lld* self, const char* lld_path);
void lld_drop(struct lld* self);
bool lld_try_detect(struct lld* self);
void lld_add_opt(struct lld* self, int opt);
void lld_add_dir(struct lld* self, const char* dir);
void lld_add_file(struct lld* self, const char* file);
void lld_set_entry(struct lld* self, const char* entry);
void lld_set_output(struct lld* self, const char* out);
int lld_run(struct lld* self);

#endif
