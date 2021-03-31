#include "scc/core/file.h"

#include "scc/core/common.h"
#include "scc/core/hash.h"

#include <stdio.h>

#include <Windows.h>
#include <FileApi.h>
#include <Shlwapi.h>
#include <string.h>

static int is_sep(int sep)
{
        return sep == '\\' || sep == '/';
}

static int has_slash(const char* s)
{
        size_t len = strlen(s);
        return len && is_sep(s[len - 1]);
}

struct pathbuf pathbuf_from_str(const char* s)
{
        struct pathbuf pb;
        strncpy(pb.buf, s, MAX_PATH_LEN);
        return pb;
}

void cwd(struct pathbuf* path)
{
        if (!GetCurrentDirectory(MAX_PATH_LEN, path->buf))
                UNREACHABLE();
        addsep(path);
}

void addsep(struct pathbuf* path)
{
        if (!has_slash(path->buf))
        {       
                size_t len = strlen(path->buf);
                if (len + 1 >= MAX_PATH_LEN)
                        UNREACHABLE();
                path->buf[len++] = PATH_DELIMETER;
                path->buf[len] = '\0';
        }
}

static void fixpath(struct pathbuf* path)
{
        char* it = path->buf;
        while (*it)
        {
                if (is_sep(*it))
                        *it = PATH_DELIMETER;
                it++;
        }
}

void join(struct pathbuf* path, const char* extra)
{
        if (!extra || !*extra)
                return;
        if (strlen(path->buf) + strlen(extra) >= MAX_PATH_LEN)
                UNREACHABLE();
        if (!has_slash(path->buf) && !is_sep(*extra))
                addsep(path);
        if (is_sep(*extra))
                extra++;
        strcat(path->buf, extra);
        fixpath(path);
}

int abspath(struct pathbuf* dst, const char* src)
{
        return !GetFullPathName(src, MAX_PATH_LEN, dst->buf, NULL)
                ? -1 : 0;
}

int isdir(const char* path)
{
        return PathIsDirectory(path);
}

int isfile(const char* path)
{
        DWORD att = GetFileAttributes(path);
        if (att == INVALID_FILE_ATTRIBUTES)
                return false;

        return !(att & FILE_ATTRIBUTE_DIRECTORY);
}

const char* pathfile(const char* path)
{
        size_t len = strlen(path);
        const char* it = path + len;
        while (it != path)
        {
                it--;
                if (is_sep(*it))
                        return it + 1;
        }
        return it;
}

const char* basename(const char* path)
{
        size_t len = strlen(path);
        if (!len)
                return path;

        const char* it = path + len - is_sep(path[len - 1]) - 1;
        while (it != path)
        {
                if (is_sep(*it))
                        return it + 1;
                it--;
        }
        return path;
}

const char* pathext(const char* path)
{
        size_t len = strlen(path);
        for (const char* it = path + len; it != path && !is_sep(*it); it--)
                if (*it == '.')
                        return it + 1;
        return path + len;
}

size_t fs_filesize(const char* path)
{
        size_t size = 0;
        HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (h != INVALID_HANDLE_VALUE) {
                LARGE_INTEGER i;
                if (GetFileSizeEx(h, &i))
                        size = (size_t)i.QuadPart;
                CloseHandle(h);
        }
        return size;
}

int fs_delfile(const char* file)
{
        return DeleteFile(file) ? 0 : -1;
}

static size_t fread_cb_read(fread_cb* self, void* buf, size_t bytes)
{
        return fread(buf, 1, bytes, self->in);
}

extern void fread_cb_init(fread_cb* self, FILE* in)
{
        self->in = in;
        read_cb_init(&self->base, &fread_cb_read);
}

static size_t fwrite_cb_write(fwrite_cb* self, const void* data, size_t bytes)
{
        return fwrite(data, 1, bytes, self->out);
}

extern void fwrite_cb_init(fwrite_cb* self, FILE* out)
{
        self->out = out;
        write_cb_init(&self->base, &fwrite_cb_write);
}

static file_entry* file_entry_new(const char* path, const char* content)
{
        file_entry* entry = alloc(sizeof(*entry));
        char* path_copy = alloc(strlen(path) + 1);
        char* content_copy = content ? alloc(strlen(content) + 1) : NULL;

        strcpy(path_copy, path);
        entry->path = path_copy;
        entry->content = content_copy;
        entry->file = NULL;
        entry->opened = false;
        entry->emulated = false;

        if (content) {
                strcpy(content_copy, content);
                entry->emulated = true;
        }

        return entry;
}

static void file_entry_delete(file_entry* entry)
{
        file_close(entry);
        dealloc(entry->path);
        dealloc(entry->content);
        dealloc(entry);
}

extern readbuf* file_open(file_entry* entry)
{
        file_close(entry);

        read_cb* read;
        if (file_emulated(entry)) {
                sread_cb_init(&entry->sread, entry->content);
                read = sread_cb_base(&entry->sread);
        } else {
                if (!(entry->file = fopen(entry->path, "rb")))
                        return NULL;

                fread_cb_init(&entry->fread, entry->file);
                read = &entry->fread.base;
        }

        entry->opened = true;
        readbuf_init(&entry->rb, read);
        return &entry->rb;
}

extern void file_close(file_entry* entry)
{
        if (!entry)
                return;

        entry->opened = false;
        if (!file_emulated(entry) && entry->file) {
                fclose(entry->file);
                entry->file = NULL;
        }
}

extern bool file_opened(const file_entry* entry)
{
        return entry->opened;
}

extern bool file_emulated(const file_entry* entry)
{
        return entry->emulated;
}

extern const char* file_get_path(const file_entry* entry)
{
        return entry->path;
}

extern const char* file_get_content(const file_entry* entry)
{
        return entry->content;
}

extern size_t file_size(const file_entry* entry)
{
        return file_emulated(entry) 
                ? strlen(file_get_content(entry))
                : fs_filesize(entry->path);
}

static file_entry* flookup_new_entry(file_lookup* self, const char* path, const char* content)
{
        file_entry* entry = file_entry_new(path, content);
        hashmap_insert(&self->lookup, strhash(path), entry);
        return entry;
}

#define VEC dirs
#define VEC_T char*
#include "scc/core/vec.inc"

extern void flookup_init(file_lookup* self)
{
        hashmap_init(&self->lookup);
        self->dirs = dirs_new();
}

extern void flookup_dispose(file_lookup* self)
{
        HASHMAP_FOREACH(&self->lookup, it)
                file_entry_delete(it.pos->value);
        hashmap_drop(&self->lookup);

        for (int i = 0; i < self->dirs->size; i++)
                dealloc(self->dirs->items[i]);
        dirs_del(self->dirs);
}

extern void flookup_add(file_lookup* self, const char* dir)
{
        assert(dir);
        char* copy = alloc(strlen(dir) + 1);
        strcpy(copy, dir);
        dirs_push(self->dirs, copy);
}

extern const char** flookup_dirs_begin(const file_lookup* self)
{
        return (const char**)self->dirs->items;
}

extern const char** flookup_dirs_end(const file_lookup* self)
{
  return (const char**)self->dirs->items + self->dirs->size;
}

extern bool file_exists(file_lookup* lookup, const char* path)
{
        return file_get(lookup, path) != NULL;
}

static file_entry* file_get_without_lookup(file_lookup* self, const char* path)
{
        struct pathbuf abs;
        if (abspath(&abs, path))
                return NULL;
        struct hashmap_entry* entry = hashmap_lookup(&self->lookup, strhash(abs.buf));
        if (entry)
                return entry->value;
        if (!isfile(abs.buf))
                return NULL;
        return flookup_new_entry(self, abs.buf, NULL);
}

static file_entry* file_get_with_lookup(file_lookup* self, const char* path)
{
        struct pathbuf abs;
        for (size_t i = 0; i < self->dirs->size; i++) {
                if (abspath(&abs, self->dirs->items[i]))
                        continue;
                join(&abs, path);

                struct hashmap_entry* entry = hashmap_lookup(&self->lookup, strhash(abs.buf));
                if (entry)
                        return entry->value;

                if (isfile(abs.buf))
                        return flookup_new_entry(self, abs.buf, NULL);
        }
        return NULL;
}

extern file_entry* file_get(file_lookup* lookup, const char* path)
{
        file_entry* entry = file_get_without_lookup(lookup, path);
        return entry ? entry : file_get_with_lookup(lookup, path);
}

extern file_entry* file_emulate(
        file_lookup* lookup, const char* path, const char* content)
{
        struct pathbuf abs;
        cwd(&abs);
        join(&abs, path);
        return flookup_new_entry(lookup, abs.buf, content);
}
