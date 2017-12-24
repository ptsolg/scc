#ifndef CLEX_H
#define CLEX_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-preprocessor.h"
#include "c-token.h"

typedef struct _clexer
{
        cpproc pp;
        creswords reswords;
} clexer;

extern void clexer_init(
        clexer* self,
        csource_manager* source_manager,
        clogger* logger,
        ccontext* context);

extern serrcode clexer_enter_source_file(clexer* self, csource* source);
extern void clexer_init_reswords(clexer* self);
extern void clexer_dispose(clexer* self);

extern ctoken* clex(clexer* self);

#ifdef __cplusplus
}
#endif

#endif // !CLEX_H
