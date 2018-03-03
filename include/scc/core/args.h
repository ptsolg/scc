#ifndef SCC_CORE_ARGS_H
#define SCC_CORE_ARGS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"

typedef struct _aparser aparser;

typedef struct
{
        const char* _prefix;
        void(*_cb)(void*, aparser*);
        void* _data;
} arg_handler;

#define ARG_HANDLER_INIT(PREFIX, CB, DATA) \
        { PREFIX, ((void(*)(void*, aparser*))CB), DATA }

extern void arg_handler_init(arg_handler* self,
        const char* prefix, void(*cb)(void*, aparser*), void* data);

typedef struct _aparser
{
        int _argc;
        int _pos;
        const char** _argv;
} aparser;

extern void aparser_init(aparser* self, int argc, const char** argv);
                
extern void aparse(aparser* self, arg_handler* handlers, size_t nhandlers, arg_handler* def);
extern int aparser_args_remain(const aparser* self);
extern const char* aparser_get_string(aparser* self);
extern errcode aparser_get_int(aparser* self, int* pint);

extern int arg_to_cmd(char* buffer, size_t buffer_size, const char* arg);
extern int argv_to_cmd(char* buffer, size_t buffer_size, int argc, const char** argv);

extern errcode execute(const char* path, int* code, int argc, const char** argv);

#ifdef __cplusplus
}
#endif

#endif
