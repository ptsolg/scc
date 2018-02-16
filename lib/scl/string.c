#include "scc/scl/string.h"
#include <stdarg.h>

extern bool streq(const char* a, const char* b, const char* ignore)
{
        const char* s[2] = { a, b };
        size_t size = strlen(ignore);

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

extern char* strprecat(char* string, const char* other)
{
        size_t size = strlen(other);
        memmove(string + size, string, strlen(string) + 1);
        memcpy(string, other, size);
        return string;
}

extern char* strcatn(char* string, ssize n, ...)
{
        va_list strings;
        va_start(strings, n);
        for (size_t i = 0; i < n; i++)
                strcat(string, va_arg(strings, char*));
        return string;
}

extern char* strprecatn(char* string, ssize n, ...)
{
        va_list strings;
        va_start(strings, n);
        for (size_t i = 0; i < n; i++)
                strprecat(string, va_arg(strings, char*));
        return string;
}

extern char* strwrap(const char* prefix, char* string, const char* suffix)
{
        strcat(string, suffix);
        strprecat(string, prefix);
        return string;
}

extern char* strend(char* string)
{
        return string + strlen(string);
}

extern const char* cstrend(const char* string)
{
        return string + strlen(string);
}

extern char* strfill(char* string, int v, ssize n)
{
        memset(string, v, n);
        string[n] = '\0';
        return string;
}