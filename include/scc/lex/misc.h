#ifndef C_MISC_H
#define C_MISC_H

#include "scc/lex/token.h"

typedef struct _c_resword_info c_resword_info;
typedef struct _tree_context tree_context;

extern void c_get_unescaped_string(char* dst, size_t dst_size, const char* string, size_t string_size);
// returns bytes written (including trailing zero)
extern size_t c_get_escaped_string(char* dst, size_t dst_size, const char* string, size_t string_size);

extern const char* c_get_token_kind_string(c_token_kind k);
extern const c_resword_info* c_get_token_info(const c_token* t);
extern const c_resword_info* c_get_token_kind_info(c_token_kind k);
extern int c_token_to_string(const tree_context* context, const c_token* tok, char* buf, size_t n);

#endif
