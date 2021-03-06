#ifndef C_SOURCE_H
#define C_SOURCE_H

#include "scc/core/file.h"
#include "scc/core/vec.h"
#include "scc/tree/common.h"

typedef struct _c_source_manager c_source_manager;
typedef struct _c_context c_context;

typedef struct _c_source
{
        file_entry* file;
        tree_location begin;
        tree_location end;
        struct u32vec* lines;
} c_source;

extern bool c_source_has(const c_source* self, tree_location loc);
// returns 0 if location is invalid
extern int c_source_get_line(const c_source* self, tree_location loc);
// returns 0 if location is invalid
extern int c_source_get_col(const c_source* self, tree_location loc);
// assumes that loc is beginning of a line
extern void c_source_save_line_loc(c_source* self, tree_location loc);
extern const char* c_source_get_name(const c_source* self);
extern file_entry* c_source_get_file(c_source* self);
extern tree_location c_source_get_loc_begin(const c_source* self);
extern tree_location c_source_get_loc_end(const c_source* self);

typedef struct _c_source_manager
{
        file_lookup* lookup;
        struct hashmap file_to_source;
        struct vec sources;
} c_source_manager;

extern void c_source_manager_init(
        c_source_manager* self, file_lookup* lookup);
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

#endif
