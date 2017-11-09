#include "scc/compiler/args.h"

extern scc_arg_handler* scc_new_arg_handler(
        allocator* alloc,
        void(*const fn)(scc_arg_handler*, aparser*),
        scc_instance* scc,
        const char* arg)
{
        scc_arg_handler* h = allocate(alloc, sizeof(*h));
        if (!h)
                return NULL;

        aparser_cb_init(&h->base, fn);
        h->scc = scc;
        h->arg = arg;
        return h;
}

extern void scc_syntax_only(scc_arg_handler* self, aparser* p)
{
        self->scc->mode = SRM_SYNTAX_ONLY;
}

extern void scc_lex_only(scc_arg_handler* self, aparser* p)
{
        self->scc->mode = SRM_LEX_ONLY;
}

extern void scc_log(scc_arg_handler* self, aparser* p)
{
        const char* log = aparser_get_string(p);
        if (!log)
        {
                fprintf(self->scc->err, "Please specify log file.\n");
                return;
        }

        char path[S_MAX_PATH_LEN];
        path_get_cd(path);
        path_join(path, log);
        FILE* f = fopen(path, "w");
        if (!f)
        {
                fprintf(self->scc->err, "Cannot open %s.\n", log);
                return;
        }

        self->scc->err = f;
}

extern void scc_o(scc_arg_handler* self, aparser* p)
{
        const char* out = aparser_get_string(p);
        if (!out)
        {
                fprintf(self->scc->err, "Please specify output file.\n");
                return;
        }

        self->scc->output = out;
}

extern void scc_i(scc_arg_handler* self, aparser* p)
{
        const char* dir = aparser_get_string(p);
        if (!dir)
        {
                fprintf(self->scc->err, "Please specify include directory.\n");
                return;
        }

        scc_add_lookup_directory(self->scc, dir);
}

extern void scc_print_eval_result(scc_arg_handler* self, aparser* p)
{
        self->scc->opts.print_eval_result = true;
}

extern void scc_print_expr_value(scc_arg_handler* self, aparser* p)
{
        self->scc->opts.print_expr_value = true;
}

extern void scc_print_expr_type(scc_arg_handler* self, aparser* p)
{
        self->scc->opts.print_expr_type = true;
}

extern void scc_print_impl_casts(scc_arg_handler* self, aparser* p)
{
        self->scc->opts.print_impl_casts = true;
}

extern void scc_force_brackets(scc_arg_handler* self, aparser* p)
{
        self->scc->opts.force_brackets = true;
}

extern void scc_float_precision(scc_arg_handler* self, aparser* p)
{
}

extern void scc_double_precision(scc_arg_handler* self, aparser* p)
{
}

extern void scc_x32(scc_arg_handler* self, aparser* p)
{
}

extern void scc_x64(scc_arg_handler* self, aparser* p)
{
        self->scc->opts.x32 = false;
}

extern void scc_default(scc_arg_handler* self, aparser* p)
{
        const char* file = aparser_get_string(p);
        scc_add_input(self->scc, file);
}
