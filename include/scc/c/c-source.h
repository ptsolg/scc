#ifndef CSOURCE_H
#define CSOURCE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/scl/file.h"
#include "scc/tree/tree-common.h"

typedef struct _csource_manager csource_manager;
typedef struct _ccontext ccontext;

typedef struct _csource
{
        list_node _node;
        file_entry* _file;
        tree_location _begin;
        tree_location _end;
        dseq_u32 _lines;
} csource;

extern bool csource_has(const csource* self, tree_location loc);
// returns 0 if location is invalid
extern int csource_get_line(const csource* self, tree_location loc);
// returns 0 if location is invalid
extern int csource_get_col(const csource* self, tree_location loc);
// assumes that loc is beginning of a line
extern serrcode csource_save_line_loc(csource* self, tree_location loc);
extern const char* csource_get_name(const csource* self);
extern tree_location csource_get_loc_begin(const csource* self);
extern tree_location csource_get_loc_end(const csource* self);

extern readbuf* csource_open(csource* self);
extern void csource_close(csource* self);

typedef struct _csource_manager
{
        file_lookup* lookup;
        strmap file_to_source;
        dseq sources;
        ccontext* context;
} csource_manager;

extern void csource_manager_init(
        csource_manager* self, file_lookup* lookup, ccontext* context);
extern void csource_manager_dispose(csource_manager* self);

extern bool csource_exists(csource_manager* self, const char* path);
extern csource* csource_get_from_file(csource_manager* self, file_entry* file);
extern csource* csource_find(csource_manager* self, const char* path);
extern csource* csource_emulate(csource_manager* self, const char* path, const char* content);

typedef struct _clocation
{
        int line;
        int column;
        const char* file;
} clocation;

extern serrcode csource_find_loc(const csource_manager* self, clocation* res, tree_location loc);

#ifdef __cplusplus
}
#endif

#endif // !CSOURCE_H
