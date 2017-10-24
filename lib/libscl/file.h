#ifndef SFILE_H
#define SFILE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "htab.h"
#include "read-write.h"
#include <stdio.h>

extern void        path_get_cd(char* path);
extern void        path_add_trailing_slash(char* path);
extern void        path_join(char* path, const char* other);
extern void        path_goto_parent_dir(char* path);
extern bool        path_has_trailing_slash(const char* path);
extern void        path_strip_file(char* path);
extern bool        path_is_dir(const char* path);
extern bool        path_is_file(const char* path);
extern bool        path_is_valid(const char* path);
extern char*       path_get_file(char* path);
extern const char* path_get_cfile(const char* path);
extern ssize       path_get_size(const char* path);
extern bool        path_is_abs(const char* path);
extern void        path_get_abs(char* abs, const char* loc);

typedef struct _fread_cb
{
        read_cb _base;
        FILE*   _in;
} fread_cb;

extern void fread_cb_init(fread_cb* self, FILE* in);

static inline read_cb* fread_cb_base(fread_cb* self)
{
        return &self->_base;
}

typedef struct _fwrite_cb
{
        write_cb _base;
        FILE*    _out;
} fwrite_cb;

extern void fwrite_cb_init(fwrite_cb* self, FILE* out);

static inline write_cb* fwrite_cb_base(fwrite_cb* self)
{
        return &self->_base;
}

#ifdef __cplusplus
}
#endif

#endif // !SFILE_H