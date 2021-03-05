#ifndef C_LANG_OPTS
#define C_LANG_OPTS

#include <stdbool.h>

typedef struct _c_lang_opts
{
        struct
        {
                bool tm_enabled;
        } ext;
} c_lang_opts;

static void c_lang_opts_init(c_lang_opts* self)
{
        self->ext.tm_enabled = false;
}

#endif