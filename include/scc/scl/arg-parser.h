#ifndef SPARSE_ARGS
#define SPARSE_ARGS

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "htab.h"

typedef struct _aparser aparser;

typedef struct
{
        const char* _prefix;
        void(*_cb)(void*, aparser*);
        void* _data;
} arg_handler;

extern void arg_handler_init(arg_handler* self,
        const char* prefix, void(*cb)(void*, aparser*), void* data);

typedef struct _aparser
{
        int _argc;
        int _pos;
        const char** _argv;
} aparser;

extern void aparser_init(aparser* self, int argc, const char** argv);
                
extern void aparse(aparser* self, arg_handler* handlers, ssize nhandlers, arg_handler* def);
extern int aparser_args_remain(const aparser* self);
extern const char* aparser_get_string(aparser* self);
extern serrcode aparser_get_int(aparser* self, int* pint);


#ifdef __cplusplus
}
#endif

#endif // !SPARSE_ARGS
