#include "scc/c/c-source.h"
#include "scc/c/c-tree.h"

extern bool csource_has(const csource* self, tree_location loc)
{
        return loc >= csource_begin(self) && loc < csource_end(self);
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
                tree_location next = csource_end(self);
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

extern readbuf* csource_open(csource* self)
{
        dseq_dispose(&self->_lines);

        read_cb* read;
        if (self->_emulated)
        {
                sread_cb_init(&self->_sread, self->_content);
                read = sread_cb_base(&self->_sread);
        }
        else
        {
                if (!(self->_file = fopen(self->_path, "r")))
                        return NULL;
                fread_cb_init(&self->_fread, self->_file);
                read = fread_cb_base(&self->_fread);
        }

        readbuf_init(&self->_rb, read);
        return &self->_rb;
}

extern void csource_close(csource* self)
{
        if (!self->_emulated)
                fclose(self->_file);
}

extern void csource_manager_init(csource_manager* self, ctree_context* context)
{
        self->alloc = tree_get_context_allocator(ctree_context_base(context));
        dseq_init_ex_ptr(&self->sources, self->alloc);
        dseq_init_ex_ptr(&self->lookup, self->alloc);
        htab_init_ex(&self->source_lookup, self->alloc);
}

static void csource_delete(allocator* alloc, csource* source)
{
        // todo
}

extern void csource_manager_dispose(csource_manager* self)
{
        for (ssize i = 0; i < dseq_size(&self->sources); i++)
                csource_delete(self->alloc, dseq_get_ptr(&self->sources, i));
        for (ssize i = 0; i < dseq_size(&self->lookup); i++)
                deallocate(self->alloc, dseq_get_ptr(&self->sources, i));

        htab_dispose(&self->source_lookup);
        dseq_dispose(&self->sources);
        dseq_dispose(&self->lookup);
}

extern serrcode csource_manager_add_lookup(csource_manager* self, const char* path)
{
        char* copy = allocate(self->alloc, strlen(path) + 1);
        if (!copy)
                return S_ERROR;

        strcpy(copy, path);
        if (S_FAILED(dseq_append_ptr(&self->lookup, copy)))
        {
                deallocate(self->alloc, copy);
                return S_ERROR;
        }
        return S_NO_ERROR;
}

extern bool csource_exists(csource_manager* self, const char* path)
{
        char abs[S_MAX_PATH_LEN];
        path_get_abs(abs, path);
        return htab_exists(&self->source_lookup, STRREF(abs));
}

static csource* _csource_new(allocator* alloc, const char* path)
{
        csource* source = allocate(alloc, sizeof(*source));
        if (!source)
                return NULL;

        source->_path = allocate(alloc, strlen(path) + 1);
        if (!source->_path)
        {
                deallocate(alloc, source);
                return NULL;
        }

        strcpy(source->_path, path);
        source->_begin = 0;
        source->_end = 0;
        source->_opened = false;
        source->_emulated = false;
        source->_content = NULL;
        source->_file = NULL;
        dseq_init_ex_u32(&source->_lines, alloc);
        return source;
}

static csource* csource_new(
        csource_manager* source_manager, const char* path, const char* content)
{
        csource* source = _csource_new(source_manager->alloc, path);
        if (!source)
                return NULL;

        dseq* sources = &source_manager->sources;
        if (dseq_size(sources))
                source->_begin = csource_end(dseq_last_ptr(sources));

        if (content)
        {
                ssize len = strlen(content) + 1; // space for eof
                source->_content = allocate(source_manager->alloc, len);
                if (!source->_content)
                {
                        csource_delete(source_manager->alloc, source);
                        return NULL;
                }

                strcpy(source->_content, content);
                source->_emulated = true;
                source->_end = source->_begin + (tree_location)len;
        }
        else
        {
                S_ASSERT(path_is_file(path));
                source->_end = source->_begin + (tree_location)path_get_size(path) + 1;
        }

        if (S_FAILED(dseq_append_ptr(sources, source)))
        {
                csource_delete(source_manager->alloc, source);
                return NULL;
        }
        return source;
}

extern csource* csource_find(csource_manager* self, const char* path)
{
        char abs[S_MAX_PATH_LEN];
        path_get_abs(abs, path);

        csource* source;
        if ((source = htab_find(&self->source_lookup, STRREF(abs))))
                return source;

        if (path_is_file(abs))
                return csource_new(self, abs, NULL);

        for (ssize i = 0; i < dseq_size(&self->lookup); i++)
        {
                path_get_abs(abs, dseq_get_ptr(&self->lookup, i));
                path_join(abs, path);
                path_fix_delimeter(abs);

                if (path_is_file(abs))
                        return csource_new(self, abs, NULL);
        }
        return NULL;
}

extern csource* csource_emulate(csource_manager* self, const char* name, const char* content)
{
        return csource_new(self, name, content);
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