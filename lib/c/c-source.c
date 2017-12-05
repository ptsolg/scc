#include "scc/c/c-source.h"
#include "scc/c/c-tree.h"

extern bool csource_has(const csource* self, tree_location loc)
{
        return loc >= csource_get_loc_begin(self) && loc < csource_get_loc_end(self);
}

extern int csource_get_line(const csource* self, tree_location loc)
{
        // todo: log(n) search
        ssize nlines = dseq_size(&self->_lines);
        if (!nlines)
                return 0;

        for (ssize i = 0; i < nlines; i++)
        {
                tree_location cur = dseq_get_u32(&self->_lines, i);
                tree_location next = csource_get_loc_end(self);
                if (i + 1 < nlines)
                        next = dseq_get_u32(&self->_lines, i + 1);

                if (loc >= cur && loc < next)
                        return (int)(i + 1);
        }

        return 0;
}

extern int csource_get_col(const csource* self, tree_location loc)
{
        int line = csource_get_line(self, loc);
        if (line == 0)
                return 0;

        tree_location line_loc = dseq_get_u32(&self->_lines, (ssize)(line - 1));
        return loc - line_loc + 1;
}

extern serrcode csource_save_line_loc(csource* self, tree_location loc)
{
        return dseq_append_u32(&self->_lines, loc);
}

extern const char* csource_get_name(const csource* self)
{
        return path_get_cfile(file_get_path(self->_file));
}

extern tree_location csource_get_loc_begin(const csource* self)
{
        return self->_begin;
}

extern tree_location csource_get_loc_end(const csource* self)
{
        return self->_end;
}

extern readbuf* csource_open(csource* self)
{
        dseq_dispose(&self->_lines);
        return file_open(self->_file);
}

extern void csource_close(csource* self)
{
        file_close(self->_file);
}

extern void csource_manager_init(
        csource_manager* self, file_lookup* lookup, ccontext* context)
{
        self->context = context;
        self->lookup = lookup;
        allocator* alloc = cget_alloc(context);
        dseq_init_ex_ptr(&self->sources, alloc);
        htab_init_ex_ptr(&self->file_to_source, alloc);
}

extern void csource_manager_dispose(csource_manager* self)
{
        HTAB_FOREACH(&self->file_to_source, it)
        {
                csource*ss = hiter_get_ptr(&it);
                csource_delete(self->context, hiter_get_ptr(&it));
        }

        htab_dispose(&self->file_to_source);
        dseq_dispose(&self->sources);
}

extern bool csource_exists(csource_manager* self, const char* path)
{
        return file_exists(self->lookup, path);
}

extern csource* csource_get_from_file(csource_manager* self, file_entry* file)
{
        if (!file)
                return NULL;

        hval ref = STRREF(file_get_path(file));
        hiter res;
        if (htab_find(&self->file_to_source, ref, &res))
                return hiter_get_ptr(&res);

        csource* source = csource_new(self->context, file);
        source->_begin = 0;
        if (dseq_size(&self->sources))
                source->_begin = csource_get_loc_end(dseq_last_ptr(&self->sources));

        source->_end = source->_begin + (tree_location)file_size(file) + 1; // space for eof
        dseq_append_ptr(&self->sources, source);
        htab_insert_ptr(&self->file_to_source, ref, source);
        return source;
}

extern csource* csource_find(csource_manager* self, const char* path)
{
        return csource_get_from_file(self, file_get(self->lookup, path));
}

extern csource* csource_emulate(csource_manager* self, const char* path, const char* content)
{
        return csource_get_from_file(self, file_emulate(self->lookup, path, content));
}

extern serrcode csource_find_loc(const csource_manager* self, clocation* res, tree_location loc)
{
        //todo: log(n) search

        for (ssize i = 0; i < dseq_size(&self->sources); i++)
        {
                csource* s = dseq_get_ptr(&self->sources, i);
                if (csource_has(s, loc))
                {
                        res->file = csource_get_name(s);
                        res->line = csource_get_line(s, loc);
                        res->column = csource_get_col(s, loc);
                        return S_NO_ERROR;
                }
        }

        res->file = "";
        res->line = 0;
        res->column = 0;
        return S_ERROR;
}