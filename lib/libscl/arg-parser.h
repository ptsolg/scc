#ifndef SPARSE_ARGS
#define SPARSE_ARGS

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "htab.h"

typedef struct _aparser_cb aparser_cb;

typedef struct _aparser
{
        int _argc;
        int _pos;
        const char** _argv;
        htab _handlers;
        aparser_cb* _default;
} aparser;

extern void aparser_init(aparser* self, int argc, const char** argv);
extern void aparser_dispose(aparser* self);
                
extern void aparse(aparser* self);
extern int aparser_args_remain(const aparser* self);
extern serrcode aparser_add_handler(aparser* self, const char* arg, aparser_cb* cb);
extern void aparser_add_default_handler(aparser* self, aparser_cb* cb);
extern bool aparser_has_handler(const aparser* self, const char* arg);

extern const char* aparser_get_string(aparser* self);
extern serrcode aparser_get_int(aparser* self, int* pint);

typedef struct _aparser_cb
{
        // argument parser will pass pointer to handler as first argument
        void(*_fn)(void*, aparser*);
} aparser_cb;

extern void aparser_cb_init(aparser_cb* self, void* callback);

#ifdef __cplusplus
}
#endif

#endif // !SPARSE_ARGS
