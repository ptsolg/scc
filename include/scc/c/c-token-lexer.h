#ifndef C_TOKEN_LEXER_H
#define C_TOKEN_LEXER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree-common.h"

typedef struct _readbuf readbuf;
typedef struct _c_reswords c_reswords;
typedef struct _c_source c_source;
typedef struct _c_source_manager c_source_manager;
typedef struct _c_logger c_logger;
typedef struct _c_context c_context;
typedef struct _c_token c_token;

typedef struct _c_token_lexer
{
        // prev, current and next
        int chars[3];
        readbuf* buf;
        bool angle_string_expected;
        bool hash_expected;
        bool in_directive;
        const c_reswords* reswords;
        c_source* source;
        c_source_manager* source_manager;
        c_logger* logger;
        c_context* context;
        tree_location loc;
        int tab_to_space;
} c_token_lexer;

extern void c_token_lexer_init(
        c_token_lexer* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context);

extern void c_token_lexer_enter_char_stream(
        c_token_lexer* self, readbuf* buf, tree_location start_loc);

extern errcode c_token_lexer_enter(c_token_lexer* self, c_source* source);

extern bool c_token_lexer_at_eof(const c_token_lexer* self);

// c99 6.4 preprocessing-token:
//      identifier
//      pp-number
//      character-constant
//      string-literal
//      punctuator
//
// we also add:
//      white-space
//      comments
extern c_token* c_token_lexer_lex_token(c_token_lexer* self);

// c99 6.4.2 identifier:
//      identifier-nondigit
//      identifier identifier-nondigit
//      identifier digit
//
// identifier-nondigit:
//      nondigit
//      other implementation-defined characters
//
// nondigit: one of
//      _ a b c d e f g h i j k l m
//      n o p q r s t u v w x y z
//      A B C D E F G H I J K L M
//      N O P Q R S T U V W X Y Z
//
// digit: one of
//      0 1 2 3 4 5 6 7 8 9
extern c_token* c_token_lexer_lex_identifier(c_token_lexer* self);

// c99 6.4.5 string-literal:
//      " s-char-sequence-opt "
//
// s-char-sequence:
//      s-char
//      s-char-sequence s-char
//
// s-char:
//      any member of the source character set except
//      the double-quote ", backslash \, or new-line character
//      escape-sequence
extern c_token* c_token_lexer_lex_string_literal(c_token_lexer* self);
extern c_token* c_token_lexer_lex_angle_string_literal(c_token_lexer* self);

// c99 6.4.4.4 character-constant:
//      ' c-char-sequence '
//
// c-char-sequence:
//      c-char
//      c-char-sequence c-char
//
// c-char:
//      any member of the source character set except
//      the single-quote ', backslash \, or new-line character
//      escape-sequence
//
// escape-sequence:
//      simple-escape-sequence
//
// simple-escape-sequence: one of
//      \' \" \? \\
//      \a \b \f \n \r \t \v
extern c_token* c_token_lexer_lex_character_constant(c_token_lexer* self);

// c99 6.4.8 pp-number:
//      digit
//      . digit
//      pp-number digit
//      pp-number identifier-nondigit
//      pp-number e sign
//      pp-number E sign
//      pp-number p sign
//      pp-number P sign
//      pp-number .
extern c_token* c_token_lexer_lex_number(c_token_lexer* self);

// 6.4.6 punctuator: one of
//      [ ] ( ) { } . ->
//      ++ -- & * + - ~ !
//      / % << >> < > <= >= == != ^ | && ||
//      ? : ; ...
//      = *= /= %= += -= <<= >>= &= ^= |=
//      , # ##
static c_token* c_token_lexer_lex_punctuator(c_token_lexer* self);


#endif