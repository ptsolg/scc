#ifndef SCC_CORE_CHAR_INFO_H
#define SCC_CORE_CHAR_INFO_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef int ascii_char;

typedef enum
{
        ACK_UNKNOWN,
        ACK_ALPHA = 1 << 0, // a-z A-Z _
        ACK_DIGIT = 1 << 1, // 0-9
        ACK_SYMBOL = 1 << 2, //
        ACK_NEWLINE = 1 << 3, // \n \r
        ACK_WSPACE = 1 << 4, // ' ' \t
} ascii_char_kind;

extern const int _ascii_char_info[256];

static inline bool is_ascii(ascii_char c)
{
        if (c < 0)
                c = -c;
        return c >= 0 && c <= 255;
}

static inline const int get_char_info(ascii_char c)
{
        assert(is_ascii(c));
        return _ascii_char_info[c];
}

static inline bool char_is_unknown(ascii_char c) { return !get_char_info(c); }
static inline bool char_is_alpha(ascii_char c) { return get_char_info(c) & ACK_ALPHA; }
static inline bool char_is_digit(ascii_char c) { return get_char_info(c) & ACK_DIGIT; }
static inline bool char_is_symbol(ascii_char c) { return get_char_info(c) & ACK_SYMBOL; }
static inline bool char_is_newline(ascii_char c) { return get_char_info(c) & ACK_NEWLINE; }
static inline bool char_is_wspace(ascii_char c) { return get_char_info(c) & ACK_WSPACE; }

static inline bool char_is_escape(ascii_char c)
{
        switch (c)
        {
                case '\n':
                case '\t':
                case '\0':
                case '\'':
                case '\"':
                case '\\':
                        return true;

                default:
                        return false;
        }
}

static inline int char_to_escape(int c)
{
        switch (c)
        {
                case 'n': return '\n';
                case 't': return '\t';
                case '0': return '\0';
                case '\'': return '\'';
                case '\"': return '\"';
                case '\\': return '\\';

                default:
                        return c;
        }
}

static inline int escape_to_char(int c)
{
        switch (c)
        {
                case '\n': return 'n';
                case '\t': return 't';
                case '\0': return '0';
                case '\'': return '\'';
                case '\"': return '\"';
                case '\\': return '\\';

                default:
                        return c;
        }
}

#ifdef __cplusplus
}
#endif

#endif
