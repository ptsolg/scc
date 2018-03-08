#ifndef C_SOURCE_H
#define C_SOURCE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/file.h"
#include "scc/tree/tree-common.h"

typedef struct _c_source_manager c_source_manager;
typedef struct _c_context c_context;

typedef struct _c_source
{
        list_node node;
        file_entry* file;
        tree_location begin;
        tree_location end;
        dseq_u32 lines;
} c_source;

extern bool c_source_has(const c_source* self, tree_location loc);
// returns 0 if location is invalid
extern int c_source_get_line(const c_source* self, tree_location loc);
// returns 0 if location is invalid
extern int c_source_get_col(const c_source* self, tree_location loc);
// assumes that loc is beginning of a line
extern errcode c_source_save_line_loc(c_source* self, tree_location loc);
extern const char* c_source_get_name(const c_source* self);
extern const char* c_source_get_path(const c_source* self);
extern tree_location c_source_get_loc_begin(const c_source* self);
extern tree_location c_source_get_loc_end(const c_source* self);

extern readbuf* c_source_open(c_source* self);
extern void c_source_close(c_source* self);

typedef struct _c_source_manager
{
        file_lookup* lookup;
        strmap file_to_source;
        dseq sources;
        c_context* context;
} c_source_manager;

extern void c_source_manager_init(
        c_source_manager* self, file_lookup* lookup, c_context* context);
extern void c_source_manager_dispose(c_source_manager* self);

extern bool c_source_exists(c_source_manager* self, const char* path);
extern c_source* c_source_get_from_file(c_source_manager* self, file_entry* file);
extern c_source* c_source_find(c_source_manager* self, const char* path);
extern c_source* c_source_emulate(c_source_manager* self, const char* path, const char* content);

typedef struct _c_location
{
        int line;
        int column;
        const char* file;
} c_location;

extern errcode c_source_find_loc(const c_source_manager* self, c_location* res, tree_location loc);

#ifdef __cplusplus
}
#endif

#endif
