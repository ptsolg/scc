#include "scc/c/c-source.h"
#include "scc/c/c-context.h"

static c_source* c_source_new(c_source_manager* manager, file_entry* entry)
{
        c_source* s = c_context_allocate(manager->context, sizeof(*s));
        s->begin = TREE_INVALID_LOC;
        s->end = TREE_INVALID_LOC;
        s->file = entry;
        list_node_init(&s->node);
        dseq_u32_init_alloc(&s->lines, c_context_get_allocator(manager->context));
        return s;
}

static void c_source_delete(c_source_manager* manager, c_source* source)
{
        if (!source)
                return;

        file_close(source->file);
        c_context_deallocate(manager->context, source);
}

extern bool c_source_has(const c_source* self, tree_location loc)
{
        return loc >= c_source_get_loc_begin(self) && loc < c_source_get_loc_end(self);
}

extern int c_source_get_line(const c_source* self, tree_location loc)
{
        // todo: log(n) search
        size_t nlines = dseq_u32_size(&self->lines);
        if (!nlines)
                return 0;

        for (size_t i = 0; i < nlines; i++)
        {
                tree_location cur = dseq_u32_get(&self->lines, i);
                tree_location next = c_source_get_loc_end(self);
                if (i + 1 < nlines)
                        next = dseq_u32_get(&self->lines, i + 1);

                if (loc >= cur && loc < next)
                        return (int)(i + 1);
        }

        return 0;
}

extern int c_source_get_col(const c_source* self, tree_location loc)
{
        int line = c_source_get_line(self, loc);
        if (line == 0)
                return 0;

        tree_location line_loc = dseq_u32_get(&self->lines, (size_t)(line - 1));
        return loc - line_loc + 1;
}

extern errcode c_source_save_line_loc(c_source* self, tree_location loc)
{
        return dseq_u32_append(&self->lines, loc);
}

extern const char* c_source_get_name(const c_source* self)
{
        return path_get_cfile(file_get_path(self->file));
}

extern const char* c_source_get_path(const c_source* self)
{
        return file_get_path(self->file);
}

extern tree_location c_source_get_loc_begin(const c_source* self)
{
        return self->begin;
}

extern tree_location c_source_get_loc_end(const c_source* self)
{
        return self->end;
}

extern readbuf* c_source_open(c_source* self)
{
        dseq_u32_dispose(&self->lines);
        return file_open(self->file);
}

extern void c_source_close(c_source* self)
{
        file_close(self->file);
}

extern void c_source_manager_init(
        c_source_manager* self, file_lookup* lookup, c_context* context)
{
        self->context = context;
        self->lookup = lookup;
        allocator* alloc = c_context_get_allocator(context);
        dseq_init_alloc(&self->sources, alloc);
        strmap_init_alloc(&self->file_to_source, alloc);
}

extern void c_source_manager_dispose(c_source_manager* self)
{
        STRMAP_FOREACH(&self->file_to_source, it)
                c_source_delete(self, *strmap_iter_value(&it));

        strmap_dispose(&self->file_to_source);
        dseq_dispose(&self->sources);
}

extern bool c_source_exists(c_source_manager* self, const char* path)
{
        return file_exists(self->lookup, path);
}

extern c_source* c_source_get_from_file(c_source_manager* self, file_entry* file)
{
        if (!file)
                return NULL;

        strref ref = STRREF(file_get_path(file));
        strmap_iter res;
        if (strmap_find(&self->file_to_source, ref, &res))
                return *strmap_iter_value(&res);

        c_source* source = c_source_new(self, file);
        source->begin = 0;
        if (dseq_size(&self->sources))
                source->begin = c_source_get_loc_end(*(dseq_end(&self->sources) - 1));

        source->end = source->begin + (tree_location)file_size(file) + 1; // space for eof
        dseq_append(&self->sources, source);
        strmap_insert(&self->file_to_source, ref, source);
        return source;
}

extern c_source* c_source_find(c_source_manager* self, const char* path)
{
        return c_source_get_from_file(self, file_get(self->lookup, path));
}

extern c_source* c_source_emulate(c_source_manager* self, const char* path, const char* content)
{
        return c_source_get_from_file(self, file_emulate(self->lookup, path, content));
}

extern errcode c_source_find_loc(const c_source_manager* self, c_location* res, tree_location loc)
{
        //todo: log(n) search

        for (size_t i = 0; i < dseq_size(&self->sources); i++)
        {
                c_source* s = dseq_get(&self->sources, i);
                if (c_source_has(s, loc))
                {
                        res->file = c_source_get_name(s);
                        res->line = c_source_get_line(s, loc);
                        res->column = c_source_get_col(s, loc);
                        return EC_NO_ERROR;
                }
        }

        res->file = "";
        res->line = 0;
        res->column = 0;
        return EC_ERROR;
}