#ifndef CSOURCE_H
#define CSOURCE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libscl/file.h>
#include <libtree/tree-common.h>

typedef struct _ctree_context ctree_context;
typedef struct _csource_manager csource_manager;

typedef struct _csource
{
        char* _path;
        tree_location _begin;
        tree_location _end;
        bool _opened;
        objgroup _lines;
        bool _emulated;
        char* _content;
        FILE* _file;
        readbuf _rb;
        union
        {
                fread_cb _fread;
                sread_cb _sread;
        };
} csource;

extern bool csource_has(const csource* self, tree_location loc);
// returns 0 if location is invalid
extern int csource_get_line(const csource* self, tree_location loc);
// returns 0 if location is invalid
extern int csource_get_col(const csource* self, tree_location loc);
// assumes that loc is beginning of a line
extern serrcode csource_save_line_loc(csource* self, tree_location loc);
                
extern readbuf* csource_open(csource* self);
extern void csource_close(csource* self);

static inline tree_location csource_begin(const csource* self)
{
        return self->_begin;
}

static inline tree_location csource_end(const csource* self)
{
        return self->_end;
}

static inline bool csource_opened(const csource* self)
{
        return self->_opened;
}

static inline const char* csource_get_path(const csource* self)
{
        return self->_path;
}

static inline const char* csource_get_name(const csource* self)
{
        return path_get_cfile(csource_get_path(self));
}

typedef struct _csource_manager
{
        htab source_lookup;
        objgroup sources;
        objgroup lookup;
        allocator* alloc;
} csource_manager;

extern void csource_manager_init(csource_manager* self, ctree_context* context);
extern void csource_manager_dispose(csource_manager* self);
extern serrcode csource_manager_add_lookup(csource_manager* self, const char* path);
extern bool csource_exists(csource_manager* self, const char* path);
extern csource* csource_find(csource_manager* self, const char* path);
extern csource* csource_emulate(csource_manager* self, const char* name, const char* content);

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
