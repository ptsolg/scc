#ifndef CPREPROCESSOR_H
#define CPREPROCESSOR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-limits.h"
#include "c-source.h"

typedef enum _ctoken_kind ctoken_kind;

typedef struct _creswords
{
        htab reswords;
        htab pp_reswords;
} creswords;

extern void creswords_init(creswords* self, ccontext* context);
extern void creswords_dispose(creswords* self);
extern void creswords_add(creswords* self, const char* string, ctoken_kind k);
extern void creswords_add_pp(creswords* self, const char* string, ctoken_kind k);
extern ctoken_kind creswords_get(const creswords* self, const char* string, ssize len);
extern ctoken_kind creswords_get_h(const creswords* self, hval h);
extern ctoken_kind creswords_get_pp(const creswords* self, const char* string, ssize len);
extern ctoken_kind creswords_get_pp_h(const creswords* self, hval h);

typedef struct _clogger clogger;
typedef struct _ctoken ctoken;

typedef struct _cpplexer
{
        // prev, current and next
        int chars[3];
        readbuf* buf;
        bool angle_string_expected;
        const creswords* reswords;
        csource* source;
        csource_manager* source_manager;
        clogger* logger;
        ccontext* context;
        tree_location loc;
        int tab_to_space;
} cpplexer;

extern void cpplexer_init(
        cpplexer* self,
        const creswords* reswords,
        csource_manager* source_manager,
        clogger* logger,
        ccontext* context);

extern serrcode cpplexer_enter_source_file(cpplexer* self, csource* source);

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
extern ctoken* cpplex_token(cpplexer* self);

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
extern ctoken* cpplex_identifier(cpplexer* self);

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
extern ctoken* cpplex_string_literal(cpplexer* self);
extern ctoken* cpplex_angle_string_literal(cpplexer* self);

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
extern ctoken* cpplex_const_char(cpplexer* self);

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
extern ctoken* cpplex_number(cpplexer* self);

// 6.4.6 punctuator: one of
// [ ] ( ) { } . ->
// ++ -- & * + - ~ !
// / % << >> < > <= >= == != ^ | && ||
// ? : ; ...
// = *= /= %= += -= <<= >>= &= ^= |=
// , # ##
static ctoken* cpplex_punctuator(cpplexer* self);

typedef struct _cmacro
{
        tree_id name;
        list_head params;
        list_head expansion;
        bool function_like;
} cmacro;

extern cmacro* cmacro_new(ccontext* context, tree_id name, bool function_like);

extern void cmacro_add_param(cmacro* self, ccontext* context, const ctoken* t);
extern void cmacro_add_expansion(cmacro* self, ccontext* context, const ctoken* t);
extern ctoken* cmacro_find_param(const cmacro* self, tree_id name);

static inline bool cmacro_is_function_like(const cmacro* self)
{
        return self->function_like;
}

static inline tree_id cmacro_get_name(const cmacro* self)
{
        return self->name;
}

typedef struct
{
        cpplexer lexer;
        csource* source;
        int nhash;
        bool hash_expected;
        bool in_directive;
} cpproc_state;

typedef struct _cpproc
{
        dseq expansion;
        cpproc_state* state;
        cpproc_state files[CMAX_INCLUDE_NESTING];
        const creswords* reswords;
        csource_manager* source_manager;
        clogger* logger;
        ccontext* context;
} cpproc;

extern void cpproc_init(
        cpproc* self,
        const creswords* reswords,
        csource_manager* source_manager,
        clogger* logger,
        ccontext* context);

extern void cpproc_dispose(cpproc* self);
extern serrcode cpproc_enter_source_file(cpproc* self, csource* source);
extern void cpproc_exit_source_file(cpproc* self);
extern ctoken* cpreprocess(cpproc* self);

#ifdef __cplusplus
}
#endif

#endif // !CPREPROCESSOR_H
