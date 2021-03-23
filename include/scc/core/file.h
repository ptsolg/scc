#ifndef SCC_CORE_FILE_H
#define SCC_CORE_FILE_H

#include "read-write.h"
#include "hashmap.h"

#include <stdio.h>

extern errcode path_get_cd(char* path);
extern errcode path_add_trailing_slash(char* path);
extern errcode path_join(char* path, const char* other);
extern void path_goto_parent_dir(char* path);
extern bool path_has_trailing_slash(const char* path);
extern void path_strip_file(char* path);
extern bool path_is_dir(const char* path);
extern bool path_is_file(const char* path);
extern bool path_is_valid(const char* path);
extern char* path_get_file(char* path);
extern const char* path_get_cfile(const char* path);
extern size_t path_get_size(const char* path);
extern bool path_is_abs(const char* path);
extern errcode path_get_abs(char* abs, const char* loc);
extern void path_fix_delimeter(char* path);
extern errcode path_change_ext(char* path, const char* ext);
extern errcode path_delete_file(const char* path);

typedef struct _fread_cb
{
        read_cb base;
        FILE* in;
} fread_cb;

extern void fread_cb_init(fread_cb* self, FILE* in);

typedef struct _fwrite_cb
{
        write_cb base;
        FILE* out;
} fwrite_cb;

extern void fwrite_cb_init(fwrite_cb* self, FILE* out);

typedef struct _file_entry
{
        char* path;
        bool opened;
        bool emulated;
        char* content;
        readbuf rb;
        FILE* file;
        union
        {
                fread_cb fread;
                sread_cb sread;
        };
} file_entry;

extern readbuf* file_open(file_entry* entry);
extern void file_close(file_entry* entry);
extern bool file_opened(const file_entry* entry);
extern bool file_emulated(const file_entry* entry);
extern const char* file_get_path(const file_entry* entry);
extern const char* file_get_content(const file_entry* entry);
extern size_t file_size(const file_entry* entry);

typedef struct _file_lookup
{
        struct hashmap lookup;
        struct dirs* dirs;
} file_lookup;

extern void flookup_init(file_lookup* self);
extern void flookup_dispose(file_lookup* self);
extern void flookup_add(file_lookup* self, const char* dir);

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


#endif
