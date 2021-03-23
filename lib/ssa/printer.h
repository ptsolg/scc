#ifndef SSA_PRINTER_H
#define SSA_PRINTER_H

#include "scc/core/read-write.h"
#include "scc/ssa/common.h"
#include "scc/tree/common.h"

typedef struct _tree_decl tree_decl;
typedef struct _ssa_context ssa_context;

#define HTAB ssa_recmap
#define HTAB_K const void*
#define HTAB_K_EMPTY (const void *)0
#define HTAB_K_DEL (const void *)1
#define HTAB_K_TO_U32(K) (unsigned)(size_t)(K)
#define HTAB_V uint
#include "scc/core/htab.inc"

typedef struct
{
        writebuf buf;
        const ssa_context* context;
        int indent_lvl;
        struct
        {
                uint tmp_id;
                uint rec_id;
                struct ssa_recmap rec_to_id;
        } llvm;
} ssa_printer;

extern void ssa_init_printer(ssa_printer* self, ssa_context* context, write_cb* cb);
extern void ssa_dispose_printer(ssa_printer* self);

extern void ssa_prints(ssa_printer* self, const char* s);
extern void ssa_printc(ssa_printer* self, int c);
extern void ssa_printf(ssa_printer* self, const char* f, ...);
extern void ssa_print_indent(ssa_printer* self);
extern void ssa_print_tree_id(ssa_printer* self, tree_id id);
extern void ssa_print_record_id(ssa_printer* self, const tree_decl* rec);
extern void ssa_gen_tmp_var(ssa_printer* self, char* buf);

#endif
