#include "scc/lex/misc.h"
#include "scc/tree/context.h"
#include "scc/lex/charset.h"
#include "scc/lex/reswords-info.h"
#include "scc/core/read-write.h"
#include <stdio.h>

extern void c_get_unescaped_string(char* dst, size_t dst_size, const char* string, size_t string_size)
{
        if (!dst_size || !string_size)
                return;

        snwrite_cb cb;
        snwrite_cb_init(&cb, dst, dst_size);
        writebuf wb;
        writebuf_init(&wb, snwrite_cb_base(&cb));

        for (size_t i = 0; i < string_size - 1; i++)
        {
                int c = string[i];
                if (c_char_is_escape(c))
                {
                        writebuf_writec(&wb, '\\');
                        writebuf_writec(&wb, c_char_from_escape(c));
                }
                else
                        writebuf_writec(&wb, c);
        }
        writebuf_flush(&wb);
        writebuf_dispose(&wb);
}

extern size_t c_get_escaped_string(char* dst, size_t dst_size, const char* string, size_t string_size)
{
        if (!dst_size || !string_size)
                return 0;

        snwrite_cb cb;
        snwrite_cb_init(&cb, dst, dst_size);
        writebuf wb;
        writebuf_init(&wb, snwrite_cb_base(&cb));

        for (size_t i = 0; i < string_size - 1; i++)
        {
                int c = string[i];
                if (c == '\\')
                        writebuf_writec(&wb, c_char_to_escape(string[++i]));
                else
                        writebuf_writec(&wb, c);

        }
        writebuf_flush(&wb);
        size_t written = writebuf_get_bytes_written(&wb);
        writebuf_dispose(&wb);
        return written;
}

extern const char* c_get_token_kind_string(c_token_kind k)
{
        return c_token_kind_to_string[k];
}

extern const c_resword_info* c_get_token_info(const c_token* t)
{
        return c_get_token_kind_info(c_token_get_kind(t));
}

extern const c_resword_info* c_get_token_kind_info(c_token_kind k)
{
        return &_c_resword_infos[k];
}

extern int c_token_to_string(const tree_context* context, const c_token* tok, char* buf, size_t n)
{
        c_token_kind k = c_token_get_kind(tok);
        const c_resword_info* info = c_get_token_kind_info(k);
        switch (k)
        {
                case CTK_ID:
                case CTK_CONST_STRING:
                case CTK_ANGLE_STRING:
                case CTK_PP_NUM:
                        return snprintf(buf, n, "%s",
                                tree_get_id_string(context, c_token_get_string(tok)));
                case CTK_CONST_CHAR:
                        return snprintf(buf, n, "'%c'", c_token_get_char(tok));
                case CTK_WSPACE:
                {
                        int c = c_token_get_spaces(tok);
                        for (int i = 0; i < c && n != 0; i++, n--)
                                buf[i] = ' ';
                        return c;
                }
                default:
                        return snprintf(buf, n, "%s", info->string);
        }
}
