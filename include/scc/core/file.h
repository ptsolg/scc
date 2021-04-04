#ifndef FILE_H
#define FILE_H

#include "buf-io.h"
#include "common.h"
#include "hashmap.h"

#define MAX_PATH_LEN 255

struct pathbuf
{
        char buf[MAX_PATH_LEN + 1];
};

struct pathbuf pathbuf_from_str(const char* s);
void cwd(struct pathbuf* path);
void addsep(struct pathbuf* path);
void join(struct pathbuf* path, const char* extra);
int abspath(struct pathbuf* dst, const char* src);
int isdir(const char* path);
int isfile(const char* path);
const char* pathfile(const char* path);
const char* basename(const char* path);
const char* pathext(const char* path);

size_t fs_filesize(const char* path);
int fs_delfile(const char* file);

typedef struct _file_entry
{
        char* path;
        int is_virtual;
        char* virtual_content;
} file_entry;

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
