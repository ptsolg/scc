#ifndef C_PREPROCESSOR_LEXER
#define C_PREPROCESSOR_LEXER

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-limits.h"
#include "c-source.h"

typedef struct _c_logger c_logger;
typedef struct _c_token c_token;

typedef struct _c_reswords
{
        strmap reswords;
        strmap pp_reswords;
} c_reswords;

extern void c_reswords_init(c_reswords* self, c_context* context);
extern void c_reswords_dispose(c_reswords* self);
extern void c_reswords_add(c_reswords* self, const char* string, int kind);
extern void c_reswords_add_pp(c_reswords* self, const char* string, int kind);
extern int c_reswords_get(const c_reswords* self, const char* string, ssize len);
extern int c_reswords_get_by_ref(const c_reswords* self, strref h);
extern int c_reswords_get_pp(const c_reswords* self, const char* string, ssize len);
extern int c_reswords_get_pp_by_ref(const c_reswords* self, strref h);

typedef struct _c_preprocessor_lexer
{
        // prev, current and next
        int chars[3];
        readbuf* buf;
        bool angle_string_expected;
        const c_reswords* reswords;
        c_source* source;
        c_source_manager* source_manager;
        c_logger* logger;
        c_context* context;
        tree_location loc;
        int tab_to_space;
} c_preprocessor_lexer;

extern void c_preprocessor_lexer_init(
        c_preprocessor_lexer* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern void c_preprocessor_lexer_enter_char_stream(
        c_preprocessor_lexer* self, readbuf* buf, tree_location start_loc);

extern serrcode c_preprocessor_lexer_enter(c_preprocessor_lexer* self, c_source* source);

extern bool c_preprocessor_lexer_at_eof(const c_preprocessor_lexer* self);

// c99 6.4 preprocessing-token:
// identifier
// pp-number
// character-constant
// string-literal
// punctuator
//
// we also add:
// white-space
// comments
extern c_token* c_preprocessor_lexer_lex_token(c_preprocessor_lexer* self);

// c99 6.4.2 identifier:
// identifier-nondigit
// identifier identifier-nondigit
// identifier digit
//
// identifier-nondigit:
// nondigit
// other implementation-defined characters
//
// nondigit: one of
// _ a b c d e f g h i j k l m
// n o p q r s t u v w x y z
// A B C D E F G H I J K L M
// N O P Q R S T U V W X Y Z
//
// digit: one of
// 0 1 2 3 4 5 6 7 8 9
extern c_token* c_preprocessor_lexer_lex_identifier(c_preprocessor_lexer* self);

// c99 6.4.5 string-literal:
// " s-char-sequence-opt "
//
// s-char-sequence:
// s-char
// s-char-sequence s-char
//
// s-char:
// any member of the source character set except
// the double-quote ", backslash \, or new-line character
// escape-sequence
extern c_token* c_preprocessor_lexer_lex_string_literal(c_preprocessor_lexer* self);
extern c_token* c_preprocessor_lexer_lex_angle_string_literal(c_preprocessor_lexer* self);

// c99 6.4.4.4 character-constant:
// ' c-char-sequence '
//
// c-char-sequence:
// c-char
// c-char-sequence c-char
//
// c-char:
// any member of the source character set except
// the single-quote ', backslash \, or new-line character
// escape-sequence
//
// escape-sequence:
// simple-escape-sequence
//
// simple-escape-sequence: one of
// \' \" \? \\
// \a \b \f \n \r \t \v
extern c_token* c_preprocessor_lexer_lex_character_constant(c_preprocessor_lexer* self);

// c99 6.4.8 pp-number:
// digit
// . digit
// pp-number digit
// pp-number identifier-nondigit
// pp-number e sign
// pp-number E sign
// pp-number p sign
// pp-number P sign
// pp-number .
extern c_token* c_preprocessor_lexer_lex_number(c_preprocessor_lexer* self);

// 6.4.6 punctuator: one of
// [ ] ( ) { } . ->
// ++ -- & * + - ~ !
// / % << >> < > <= >= == != ^ | && ||
// ? : ; ...
// = *= /= %= += -= <<= >>= &= ^= |=
// , # ##
static c_token* c_preprocessor_lexer_lex_punctuator(c_preprocessor_lexer* self);

#ifdef __cplusplus
}
#endif

#endif
