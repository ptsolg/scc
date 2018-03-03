#ifndef SFILE_H
#define SFILE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "read-write.h"
#include "strmap.h"
#include "dseq-instance.h"
#include <stdio.h>

extern serrcode path_get_cd(char* path);
extern serrcode path_add_trailing_slash(char* path);
extern serrcode path_join(char* path, const char* other);
extern void path_goto_parent_dir(char* path);
extern bool path_has_trailing_slash(const char* path);
extern void path_strip_file(char* path);
extern bool path_is_dir(const char* path);
extern bool path_is_file(const char* path);
extern bool path_is_valid(const char* path);
extern char* path_get_file(char* path);
extern const char* path_get_cfile(const char* path);
extern ssize path_get_size(const char* path);
extern bool path_is_abs(const char* path);
extern serrcode path_get_abs(char* abs, const char* loc);
extern void path_fix_delimeter(char* path);
extern serrcode path_change_ext(char* path, const char* ext);
extern serrcode path_delete_file(const char* path);

typedef struct _fread_cb
{
        read_cb _base;
        FILE* _in;
} fread_cb;

extern void fread_cb_init(fread_cb* self, FILE* in);

static inline read_cb* fread_cb_base(fread_cb* self)
{
        return &self->_base;
}

typedef struct _fwrite_cb
{
        write_cb _base;
        FILE* _out;
} fwrite_cb;

extern void fwrite_cb_init(fwrite_cb* self, FILE* out);

static inline write_cb* fwrite_cb_base(fwrite_cb* self)
{
        return &self->_base;
}

typedef struct _file_entry
{
        char* _path;
        bool _opened;
        bool _emulated;
        char* _content;
        readbuf _rb;
        FILE* _file;
        union
        {
                fread_cb _fread;
                sread_cb _sread;
        };
} file_entry;

extern readbuf* file_open(file_entry* entry);
extern void file_close(file_entry* entry);
extern bool file_opened(const file_entry* entry);
extern bool file_emulated(const file_entry* entry);
extern const char* file_get_path(const file_entry* entry);
extern const char* file_get_content(const file_entry* entry);
extern ssize file_size(const file_entry* entry);

typedef struct _file_lookup
{
        strmap _lookup;
        dseq _dirs;
        allocator* _alloc;
} file_lookup;

extern void flookup_init(file_lookup* self);
extern void flookup_init_ex(file_lookup* self, allocator* alloc);
extern void flookup_dispose(file_lookup* self);
extern serrcode flookup_add(file_lookup* self, const char* dir);

extern const char** flookup_dirs_begin(const file_lookup* self);
extern const char** flookup_dirs_end(const file_lookup* self);

#define FLOOKUP_FOREACH_DIR(PLOOKUP, ITNAME, ENDNAME)\
        for (const char** ITNAME = flookup_dirs_begin(PLOOKUP), \
                **ENDNAME = flookup_dirs_end(PLOOKUP); \
                ITNAME != ENDNAME; ITNAME++)

extern bool file_exists(file_lookup* lookup, const char* path);
extern file_entry* file_get(file_lookup* lookup, const char* path);
extern file_entry* file_emulate(
        file_lookup* lookup, const char* path, const char* content);


#ifdef __cplusplus
}
#endif

#endif // !SFILE_H
