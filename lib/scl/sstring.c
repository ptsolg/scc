#include "scc/scl/sstring.h"
#include <stdarg.h>

typedef struct
{
        ssize len;
        suint8 data[0];
} strentry_impl;

extern void strpool_init(strpool* self)
{
        strpool_init_ex(self, STDALLOC);
}

extern void strpool_init_ex(strpool* self, allocator* alloc)
{
        htab_init_ex_ptr(&self->_strings, alloc);
        bump_ptr_allocator_init_ex(&self->_alloc, alloc);
}

extern void strpool_dispose(strpool* self)
{
        htab_dispose(&self->_strings);
        bump_ptr_allocator_dispose(&self->_alloc);
}

extern bool strpool_get(const strpool* self, strref ref, strentry* result)
{
        result->data = NULL;
        result->len = 0;

        hiter it;
        if (!htab_find(&self->_strings, ref, &it))
                return false;

        strentry_impl* entry = hiter_get_ptr(&it);
        result->data = entry->data;
        result->len = entry->len;
        return true;
}

extern bool strpooled(const strpool* self, strref ref)
{
        return htab_exists(&self->_strings, ref);
}

extern strref strpool_insert(strpool* self, const void* data, ssize len)
{
        strref r = STRREFL(data, len);

        strentry pooled;
        while (strpool_get(self, r, &pooled))
        {
                if (pooled.len == len && memcmp(pooled.data, data, len) == 0)
                        return r;
                r += s_mix32(r);
        }

        if (S_FAILED(htab_reserve(&self->_strings, r)))
                return STRREF_INVALID;

        strentry_impl* copy = bump_ptr_allocate(&self->_alloc,
                sizeof(strentry_impl) + len + 1);
        if (!copy)
        {
                htab_erase(&self->_strings, r);
                return STRREF_INVALID;
        }

        copy->len = len;
        memcpy(copy->data, data, len);
        copy->data[len] = '\0';
        htab_insert_ptr(&self->_strings, r, copy);
        return r;
}

extern ssize sstrlen(const char* string)
{
        return strlen(string);
}

extern bool sstreq(const char* a, const char* b, const char* ignore)
{
        const char* s[2] = { a, b };
        size_t size = sstrlen(ignore);

        while (true)
        {
                for (int i = 0; i < 2; i++)
                {
                        bool again = false;
                        do
                        {
                                again = false;
                                for (size_t j = 0; j < size; j++)
                                        while (*s[i] && *s[i] == ignore[j])
                                        {
                                                s[i]++;
                                                again = true;
                                        }
                        } while (again);

                }
                if (*s[0] != *s[1])
                        return false;
                if (!*s[0] && !*s[1])
                        return true;
                s[0]++;
                s[1]++;
        }
        return false;
}

extern char* sstrprecat(char* string, const char* other)
{
        size_t size = strlen(other);
        memmove(string + size, string, strlen(string) + 1);
        memcpy(string, other, size);
        return string;
}

extern char* sstrcatn(char* string, ssize n, ...)
{
        va_list strings;
        va_start(strings, n);
        for (size_t i = 0; i < n; i++)
                strcat(string, va_arg(strings, char*));
        return string;
}

extern char* sstrprecatn(char* string, ssize n, ...)
{
        va_list strings;
        va_start(strings, n);
        for (size_t i = 0; i < n; i++)
                sstrprecat(string, va_arg(strings, char*));
        return string;
}

extern char* sstrwrap(const char* prefix, char* string, const char* suffix)
{
        strcat(string, suffix);
        sstrprecat(string, prefix);
        return string;
}

extern char* sstrend(char* string)
{
        return string + sstrlen(string);
}

extern const char* scstrend(const char* string)
{
        return string + sstrlen(string);
}

extern char* sstrfill(char* string, int v, ssize n)
{
        memset(string, v, n);
        string[n] = '\0';
        return string;
}