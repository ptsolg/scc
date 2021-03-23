#ifndef C_RESWORDS_H
#define C_RESWORDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/hashmap.h"

typedef struct _c_context c_context;

typedef struct _c_reswords
{
        struct hashmap reswords;
        struct hashmap pp_reswords;
} c_reswords;

extern void c_reswords_init(c_reswords* self, c_context* context);
extern void c_reswords_dispose(c_reswords* self);
extern void c_reswords_add_resword(c_reswords* self, const char* string, int kind);
extern void c_reswords_add_pp_resword(c_reswords* self, const char* string, int kind);
extern int c_reswords_get_resword(const c_reswords* self, const char* string, size_t len);
extern int c_reswords_get_resword_by_ref(const c_reswords* self, unsigned ref);
extern int c_reswords_get_pp_resword(const c_reswords* self, const char* string, size_t len);
extern int c_reswords_get_pp_resword_by_ref(const c_reswords* self, unsigned ref);

#ifdef __cplusplus
}
#endif

#endif
