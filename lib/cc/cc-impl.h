#ifndef CC_IMPL_H
#define CC_IMPL_H

#include "scc/cc/cc.h"

extern void cc_error(cc_instance* self, const char* format, ...);
extern void cc_unable_to_open(cc_instance* self, const char* path);
extern void cc_file_doesnt_exit(cc_instance* self, const char* file);

extern serrcode cc_dump_tokens(cc_instance* self);
extern serrcode cc_dump_tree(cc_instance* self);
extern serrcode cc_perform_syntax_analysis(cc_instance* self);
extern serrcode cc_generate_obj(cc_instance* self);
extern serrcode cc_generate_asm(cc_instance* self);
extern serrcode cc_generate_ssa(cc_instance* self);
extern serrcode cc_generate_llvm_ir(cc_instance* self);
extern serrcode cc_generate_exec(cc_instance* self);

#endif