#ifndef C_CHARSET_H
#define C_CHARSET_H

#include <assert.h>
#include <stdbool.h>

typedef enum
{
        CCK_UNKNOWN = 0,
        CCK_LETTER = 1,
        CCK_DIGIT = 2,
        CCK_PUNCTUATOR = 4,
        CCK_SPACE = 8,
} c_char_kind;

extern const int c_char_info_table[256];

static inline bool c_char_is(int c, c_char_kind k)
{
        assert(c >= 0 && c <= 255);
        return c_char_info_table[c] & k;
}

static inline bool c_char_is_unknown(int c)
{
        return !c_char_info_table[c];
}

static inline bool c_char_is_letter(int c)
{
        return c_char_is(c, CCK_LETTER);
}

static inline bool c_char_is_digit(int c)
{
        return c_char_is(c, CCK_DIGIT);
}

static inline bool c_char_is_punctuator(int c)
{
        return c_char_is(c, CCK_PUNCTUATOR);
}

static inline bool c_char_is_space(int c)
{
        return c_char_is(c, CCK_SPACE);
}

static inline bool c_char_is_escape(int c)
{
        switch (c)
        {
                case '\n':
                case '\t':
                case '\r':
                case '\0':
                case '\'':
                case '\"':
                case '\\':
                        return true;

                default:
                        return false;
        }
}

static inline int c_char_to_escape(int c)
{
        switch (c)
        {
                case 'n': return '\n';
                case 't': return '\t';
                case 'r': return '\r';
                case '0': return '\0';
                case '\'': return '\'';
                case '\"': return '\"';
                case '\\': return '\\';

                default:
                        return c;
        }
}

static inline int c_char_from_escape(int c)
{
        switch (c)
        {
                case '\n': return 'n';
                case '\t': return 't';
                case '\r': return 'r';
                case '\0': return '0';
                case '\'': return '\'';
                case '\"': return '\"';
                case '\\': return '\\';

                default:
                        return c;
        }
}

#endif