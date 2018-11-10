#include "printer.h"
#include "scc/ssa/context.h"
#include "scc/tree/context.h"
#include <stdarg.h>
#include <stdio.h>

extern void ssa_init_printer(ssa_printer* self, ssa_context* context, write_cb* cb)
{
        writebuf_init(&self->buf, cb);
        self->context = context;
        self->indent_lvl = 0;
        self->llvm.tmp_id = 0;
        self->llvm.rec_id = 0;
        ssa_recmap_init_ex(&self->llvm.rec_to_id, ssa_get_alloc(context));
}

extern void ssa_dispose_printer(ssa_printer* self)
{
        writebuf_flush(&self->buf);
        ssa_recmap_dispose(&self->llvm.rec_to_id);
}

extern void ssa_prints(ssa_printer* self, const char* s)
{
        if (s)
                writebuf_writes(&self->buf, s);
}

extern void ssa_printc(ssa_printer* self, int c)
{
        writebuf_writec(&self->buf, c);
}

extern void ssa_printf(ssa_printer* self, const char* f, ...)
{
        char buf[1024];

        va_list args;
        va_start(args, f);
        vsnprintf(buf, 1024, f, args);

        ssa_prints(self, buf);
}

extern void ssa_print_indent(ssa_printer* self)
{
        int indent = self->indent_lvl;
        while (indent--)
                ssa_prints(self, "    ");
}

extern void ssa_print_endl(ssa_printer* self)
{
        ssa_printc(self, '\n');
}

extern void ssa_print_tree_id(ssa_printer* self, tree_id id)
{
        ssa_prints(self, tree_get_id_string(self->context->tree, id));
}

extern void ssa_print_record_id(ssa_printer* self, const tree_decl* rec)
{
        uint id;
        ssa_recmap_entry* entry = ssa_recmap_lookup(&self->llvm.rec_to_id, rec);
        if (entry)
                id = entry->value;
        else
        {
                id = self->llvm.rec_id++;
                ssa_recmap_insert(&self->llvm.rec_to_id, rec, id);
        }
        ssa_printf(self, "%u", id);
}

extern void ssa_gen_tmp_var(ssa_printer* self, char* buf)
{
        sprintf(buf, "%%tmp.%u", self->llvm.tmp_id++);
}