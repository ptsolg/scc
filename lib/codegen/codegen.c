#include "scc/codegen/codegen.h"
#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-opt.h"
#include "scc/ssa/ssa-printer.h"
#include "scc/codegen/llvm-printer.h"

extern errcode codegen_module(
        write_cb* write,
        ssa_context* context,
        const tree_module* module,
        codegen_output_kind output,
        const ssa_optimizer_opts* opts)
{
        ssaizer sr;
        ssaizer_init(&sr, context);
        ssa_module* sm = ssaize_module(&sr, module);
        ssaizer_dispose(&sr);
        if (!sm)
                return EC_ERROR;

        ssa_optimize(context, sm, opts);
        if (output == CGOK_SSA)
        {
                ssa_printer printer;
                ssa_init_printer(&printer, write, context);
                ssa_print_module(&printer, sm);
                ssa_dispose_printer(&printer);
        }
        else if (output == CGOK_LLVM)
        {
                llvm_printer printer;
                llvm_printer_init(&printer, write, context);
                llvm_printer_emit_module(&printer, sm);
                llvm_printer_dispose(&printer);
        }
        else
                return EC_ERROR;

        return EC_NO_ERROR;
}