#include "scc/core/file.h"
#include "scc/core/string.h"
#include <stdio.h>

#if OS_WIN
#include <Windows.h>
#include <FileApi.h>
#include <Shlwapi.h>

#if COMPILER_MSVC
#pragma comment(lib, "shlwapi.lib")
#else
#error //todo
#endif

#elif OS_OSX
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h> // realpath
#else
#error todo
#endif

extern errcode path_get_cd(char* path)
{
#if OS_WIN
        if (!GetCurrentDirectory(MAX_PATH_LEN, path))
                return EC_ERROR;
#elif OS_OSX
        if (!getcwd(path, MAX_PATH_LEN))
                return EC_ERROR;
#else
#error todo
#endif
        return path_add_trailing_slash(path);
}

extern errcode path_add_trailing_slash(char* path)
{
        if (!path_has_trailing_slash(path))
        {
                size_t len = strlen(path);
                if (len + 1 >= MAX_PATH_LEN)
                        return EC_ERROR;

                path[len++] = PATH_DELIMETER;
                path[len] = '\0';
        }
        return EC_NO_ERROR;
}

extern errcode path_join(char* path, const char* other)
{
        if (strlen(path) + strlen(other) >= MAX_PATH_LEN)
                return EC_ERROR;

        if (*path && !path_has_trailing_slash(path))
                if (EC_FAILED(path_add_trailing_slash(path)))
                        return EC_ERROR;

        strcat(path, other);
        return EC_NO_ERROR;
}

extern void path_goto_parent_dir(char* path)
{
        char* file = path_get_file(path);
        if (file == path)
        {
                *path = '\0';
                return;
        }
        *(file - 1) = '\0';
        path_strip_file(path);
}

extern bool path_has_trailing_slash(const char* path)
{
        size_t len = strlen(path);
        return len && path[len - 1] == PATH_DELIMETER;
}

extern void path_strip_file(char* path)
{
        char* file = path_get_file(path);
        *file = '\0';
}

extern bool path_is_dir(const char* path)
{
#if OS_WIN
        return PathIsDirectory(path);
#elif OS_OSX
        struct stat s;
        if (stat(path, &s) != 0)
                return false;
        return S_ISDIR(s.st_mode);
#else
#error
#endif
}

extern bool path_is_file(const char* path)
{
#if OS_WIN
        DWORD att = GetFileAttributes(path);
        if (att == INVALID_FILE_ATTRIBUTES)
                return false;

        return !(att & FILE_ATTRIBUTE_DIRECTORY);
#elif OS_OSX
        struct stat s;
        if (stat(path, &s) != 0)
                return false;
        return S_ISREG(s.st_mode);
#else
#error
#endif
}

extern bool path_is_valid(const char* path)
{
        return path_is_file(path) || path_is_dir(path);
}

extern char* path_get_file(char* path)
{
        char* end = strend(path);
        while (end != path)
        {
                end--;
                if (*end == PATH_DELIMETER)
                        return end + 1;
        }
        return end;
}

extern const char* path_get_cfile(const char* path)
{
        const char* end = cstrend(path);
        while (end != path)
        {
                end--;
                if (*end == PATH_DELIMETER)
                        return end + 1;
        }
        return end;
}

extern size_t path_get_size(const char* path)
{
        size_t res = 0;
#if OS_WIN
        HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
                LARGE_INTEGER i;
                if (GetFileSizeEx(h, &i))
                        res = (size_t)i.QuadPart;
                CloseHandle(h);
        }
        return res;

#elif OS_OSX
        struct stat st;
        if (stat(path, &st) != 0)
                return 0;
        return st.st_size;
#else
#error todo
#endif
}

extern bool path_is_abs(const char* path)
{
#if OS_WIN
        return !PathIsRelative(path);
#elif OS_OSX
        return false;
#else
#error todo
#endif
}

extern void path_fix_delimeter(char* path)
{
        while(1)
        {
               int c = *path;
               if (!c)
                       return;

               if (c == '\\' || c == '/')
                       *path = PATH_DELIMETER;
               path++;
        }
}

extern errcode path_change_ext(char* path, const char* ext)
{
        path_fix_delimeter(path);
        char* end = strend(path);
        char* pos = end;
        while (pos != path && *pos != '.' && *pos != PATH_DELIMETER)
                pos--;

        if (pos == path || *pos == PATH_DELIMETER)
                pos = end;

        size_t rem = MAX_PATH_LEN - strlen(path);
        if (strlen(ext) + 1 >= rem)
                return EC_ERROR;

        *pos++ = '.';
        strcpy(pos, ext);
        return EC_NO_ERROR;
}

extern errcode path_delete_file(const char* path)
{
#if OS_WIN
        return DeleteFile(path) ? EC_NO_ERROR : EC_ERROR;
#elif OS_OSX
        return remove(path) == 0 ? EC_NO_ERROR : EC_ERROR;
#endif
}

extern errcode path_get_abs(char* abs, const char* loc)
{
        if (path_is_abs(loc))
        {
                strcpy(abs, loc);
                return EC_NO_ERROR;
        }

#if OS_WIN
        if (!GetFullPathName(loc, MAX_PATH_LEN, abs, NULL))
                return EC_ERROR;
#elif OS_OSX
        if (!realpath(loc, abs))
                return EC_ERROR;
#else
#error todo
#endif

        return EC_NO_ERROR;
}

static size_t fread_cb_read(fread_cb* self, void* buf, size_t bytes)
{
        return fread(buf, 1, bytes, self->_in);
}

extern void fread_cb_init(fread_cb* self, FILE* in)
{
        self->_in = in;
        read_cb_init(&self->_base, &fread_cb_read);
}

static size_t fwrite_cb_write(fwrite_cb* self, const void* data, size_t bytes)
{
        return fwrite(data, 1, bytes, self->_out);
}

extern void fwrite_cb_init(fwrite_cb* self, FILE* out)
{
        self->_out = out;
        write_cb_init(&self->_base, &fwrite_cb_write);
}

static file_entry* file_entry_new(allocator* alloc, const char* path, const char* content)
{
        file_entry* entry = NULL;
        char* path_copy = NULL;
        char* content_copy = NULL;

        if (!(entry = allocate(alloc, sizeof(*entry))))
                goto error;
        if (!(path_copy = allocate(alloc, strlen(path) + 1)))
                goto error;
        if (content && !(content_copy = allocate(alloc, strlen(content) + 1)))
                goto error;

        strcpy(path_copy, path);
        entry->_path = path_copy;
        entry->_content = content_copy;
        entry->_file = NULL;
        entry->_opened = false;
        entry->_emulated = false;

        if (content)
        {
                strcpy(content_copy, content);
                entry->_emulated = true;
        }

        return entry;

error:
        deallocate(alloc, entry);
        deallocate(alloc, path_copy);
        deallocate(alloc, content_copy);
        return NULL;
}

static void file_entry_delete(allocator* alloc, file_entry* entry)
{
        file_close(entry);
        deallocate(alloc, entry->_path);
        deallocate(alloc, entry->_content);
        deallocate(alloc, entry);
}

extern readbuf* file_open(file_entry* entry)
{
        file_close(entry);

        read_cb* read;
        if (file_emulated(entry))
        {
                sread_cb_init(&entry->_sread, entry->_content);
                read = sread_cb_base(&entry->_sread);
        }
        else
        {
                if (!(entry->_file = fopen(entry->_path, "rb")))
                        return NULL;

                fread_cb_init(&entry->_fread, entry->_file);
                read = fread_cb_base(&entry->_fread);
        }

        entry->_opened = true;
        readbuf_init(&entry->_rb, read);
        return &entry->_rb;
}

extern void file_close(file_entry* entry)
{
        if (!entry)
                return;

        entry->_opened = false;
        if (!file_emulated(entry) && entry->_file)
        {
                fclose(entry->_file);
                entry->_file = NULL;
        }
}

extern bool file_opened(const file_entry* entry)
{
        return entry->_opened;
}

extern bool file_emulated(const file_entry* entry)
{
        return entry->_emulated;
}

extern const char* file_get_path(const file_entry* entry)
{
        return entry->_path;
}

extern const char* file_get_content(const file_entry* entry)
{
        return entry->_content;
}

extern size_t file_size(const file_entry* entry)
{
        return file_emulated(entry)
                ? strlen(file_get_content(entry))
                : path_get_size(file_get_path(entry));
}

static file_entry* flookup_new_entry(file_lookup* self, const char* path, const char* content)
{
        file_entry* entry = file_entry_new(self->_alloc, path, content);
        if (!entry)
                return NULL;

        if (EC_FAILED(strmap_insert(&self->_lookup, STRREF(path), entry)))
        {
                file_entry_delete(self->_alloc, entry);
                return NULL;
        }

        return entry;
}

extern void flookup_init(file_lookup* self)
{
        flookup_init_ex(self, STDALLOC);
}

extern void flookup_init_ex(file_lookup* self, allocator* alloc)
{
        self->_alloc = alloc;
        strmap_init_alloc(&self->_lookup, alloc);
        dseq_init_alloc(&self->_dirs, alloc);
}

extern void flookup_dispose(file_lookup* self)
{
        STRMAP_FOREACH(&self->_lookup, it)
                file_entry_delete(self->_alloc, *strmap_iter_value(&it));
        strmap_dispose(&self->_lookup);

        for (void** it = dseq_begin(&self->_dirs);
                it != dseq_end(&self->_dirs); it++)
        {
                deallocate(self->_alloc, *it);
        }
        dseq_dispose(&self->_dirs);
}

extern errcode flookup_add(file_lookup* self, const char* dir)
{
        assert(dir);
        char* copy = allocate(self->_alloc, strlen(dir) + 1);
        if (!copy)
                return EC_ERROR;

        strcpy(copy, dir);
        return dseq_append(&self->_dirs, copy);
}

extern const char** flookup_dirs_begin(const file_lookup* self)
{
        return (const char**)dseq_begin(&self->_dirs);
}

extern const char** flookup_dirs_end(const file_lookup* self)
{
        return (const char**)dseq_end(&self->_dirs);
}

extern bool file_exists(file_lookup* lookup, const char* path)
{
        return file_get(lookup, path) != NULL;
}

static file_entry* file_get_without_lookup(file_lookup* self, const char* path)
{
        char abs[MAX_PATH_LEN + 1];
        if (EC_FAILED(path_get_abs(abs, path)))
                return NULL;

        strmap_iter res;
        if (strmap_find(&self->_lookup, STRREF(abs), &res))
                return *strmap_iter_value(&res);

        if (!path_is_file(abs))
                return NULL;

        return flookup_new_entry(self, abs, NULL);
}

static file_entry* file_get_with_lookup(file_lookup* self, const char* path)
{
        char abs[MAX_PATH_LEN + 1];
        for (size_t i = 0; i < dseq_size(&self->_dirs); i++)
        {
                if (EC_FAILED(path_get_abs(abs, dseq_get(&self->_dirs, i))))
                        continue;
                if (EC_FAILED(path_join(abs, path)))
                        continue;
                path_fix_delimeter(abs);

                strmap_iter res;
                if (strmap_find(&self->_lookup, STRREF(abs), &res))
                        return *strmap_iter_value(&res);

                if (path_is_file(abs))
                        return flookup_new_entry(self, abs, NULL);
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
        char abs[MAX_PATH_LEN + 1];
        if (EC_FAILED(path_get_cd(abs)))
                return NULL;
        if (EC_FAILED(path_join(abs, path)))
                return NULL;

        return flookup_new_entry(lookup, abs, content);
}