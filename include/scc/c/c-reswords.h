#ifndef CRESWORDS_H
#define CRESWORDS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/scl/common.h"
#include "c-token-kind.h"

S_STATIC_ASSERT(CTK_TOTAL_SIZE == 108, "Should update creswords table.");

typedef struct _cresword_info
{
        const char* string;
        const char* desription;
        const char* kind;
} cresword_info;

#define CRESWORD(s, d, k) {s, d, k}
#define CRK_TOKEN "token"
#define CRK_LITERAL "literal"
#define CRK_EMPTY ""

static const cresword_info _creswords[CTK_TOTAL_SIZE] =
{
        CRESWORD("", "unknown", CRK_TOKEN), // CTK_UNKNOWN
        CRESWORD("", "end of file", CRK_EMPTY), // CTK_EOF
        CRESWORD("", "end of line", CRK_EMPTY), // CTK_EOL
        CRESWORD("", "white space", CRK_EMPTY), // CTK_WSPACE
        CRESWORD("", "commetary", CRK_EMPTY), // CTK_COMMENT

        CRESWORD("char", "'char'", CRK_TOKEN),
        CRESWORD("short", "'short'", CRK_TOKEN),
        CRESWORD("int", "'int'", CRK_TOKEN),
        CRESWORD("long", "'long'", CRK_TOKEN),
        CRESWORD("float", "'float'", CRK_TOKEN),
        CRESWORD("double", "'double'", CRK_TOKEN),
        CRESWORD("signed", "'signed'", CRK_TOKEN),
        CRESWORD("unsigned", "'unsigned'", CRK_TOKEN),
        CRESWORD("void", "'void'", CRK_TOKEN),

        CRESWORD("struct", "'struct'", CRK_TOKEN),
        CRESWORD("union", "'union'", CRK_TOKEN),
        CRESWORD("enum", "'enum'", CRK_TOKEN),

        CRESWORD("if", "'if'", CRK_TOKEN),
        CRESWORD("else", "'else'", CRK_TOKEN),
        CRESWORD("while", "'while'", CRK_TOKEN),
        CRESWORD("for", "'for'", CRK_TOKEN),
        CRESWORD("do", "'do'", CRK_TOKEN),
        CRESWORD("break", "'break'", CRK_TOKEN),
        CRESWORD("continue", "'continue'", CRK_TOKEN),
        CRESWORD("return", "'return'", CRK_TOKEN),
        CRESWORD("goto", "'goto'", CRK_TOKEN),
        CRESWORD("switch", "'switch'", CRK_TOKEN),
        CRESWORD("default", "'default'", CRK_TOKEN),
        CRESWORD("case", "'case'", CRK_TOKEN),

        CRESWORD("auto", "'auto'", CRK_TOKEN),
        CRESWORD("register", "'register'", CRK_TOKEN),
        CRESWORD("static", "'static'", CRK_TOKEN),
        CRESWORD("extern", "'extern'", CRK_TOKEN),
        CRESWORD("inline", "'inline'", CRK_TOKEN),
        CRESWORD("const", "'const'", CRK_TOKEN),
        CRESWORD("volatile", "'volatile'", CRK_TOKEN),
        CRESWORD("restrict", "'restrict'", CRK_TOKEN),

        CRESWORD("sizeof", "'sizeof'", CRK_TOKEN),
        CRESWORD("typedef", "'typedef'", CRK_TOKEN),
        CRESWORD("", "identifier", CRK_EMPTY), // CTK_ID

        CRESWORD("{", "'{'", CRK_TOKEN),
        CRESWORD("}", "'}'", CRK_TOKEN),
        CRESWORD("[", "'['", CRK_TOKEN),
        CRESWORD("]", "']'", CRK_TOKEN),
        CRESWORD("(", "'('", CRK_TOKEN),
        CRESWORD(")", "')'", CRK_TOKEN),
        CRESWORD(".", "'.'", CRK_TOKEN),
        CRESWORD("->", "'->'", CRK_TOKEN),
        CRESWORD("<", "'<'", CRK_TOKEN),
        CRESWORD(">", "'>'", CRK_TOKEN),
        CRESWORD(">=", "'>='", CRK_TOKEN),
        CRESWORD("<=", "'<='", CRK_TOKEN),
        CRESWORD("==", "'=='", CRK_TOKEN),
        CRESWORD("!=", "'!='", CRK_TOKEN),
        CRESWORD("&&", "'&&'", CRK_TOKEN),
        CRESWORD("||", "'||'", CRK_TOKEN),
        CRESWORD("!", "'!'", CRK_TOKEN),
        CRESWORD("~", "'~'", CRK_TOKEN),
        CRESWORD("^", "'^'", CRK_TOKEN),
        CRESWORD("&", "'&'", CRK_TOKEN),
        CRESWORD("|", "'|'", CRK_TOKEN),
        CRESWORD("+", "'+'", CRK_TOKEN),
        CRESWORD("-", "'-'", CRK_TOKEN),
        CRESWORD("*", "'*'", CRK_TOKEN),
        CRESWORD("/", "'/'", CRK_TOKEN),
        CRESWORD("%", "'%'", CRK_TOKEN),
        CRESWORD("--", "'--'", CRK_TOKEN),
        CRESWORD("++", "'++'", CRK_TOKEN),
        CRESWORD("<<", "'<<'", CRK_TOKEN),
        CRESWORD(">>", "'>>'", CRK_TOKEN),
        CRESWORD("=", "'='", CRK_TOKEN),
        CRESWORD("^=", "'^='", CRK_TOKEN),
        CRESWORD("&=", "'&='", CRK_TOKEN),
        CRESWORD("|=", "'|='", CRK_TOKEN),
        CRESWORD("+=", "'+='", CRK_TOKEN),
        CRESWORD("-=", "'-='", CRK_TOKEN),
        CRESWORD("*=", "'*='", CRK_TOKEN),
        CRESWORD("/=", "'/='", CRK_TOKEN),
        CRESWORD("%=", "'%='", CRK_TOKEN),
        CRESWORD("<<=", "'<<='", CRK_TOKEN),
        CRESWORD(">>=", "'>>='", CRK_TOKEN),
        CRESWORD(":", "':'", CRK_TOKEN),
        CRESWORD(",", "','", CRK_TOKEN),
        CRESWORD("?", "'?'", CRK_TOKEN),
        CRESWORD(";", "';'", CRK_TOKEN),
        CRESWORD("...", "'...'", CRK_TOKEN),
        CRESWORD("#", "'#'", CRK_TOKEN),
        CRESWORD("##", "'##'", CRK_TOKEN),

        CRESWORD("", "integer", CRK_LITERAL), // CTK_CONST_INT
        CRESWORD("", "float", CRK_LITERAL), // CTK_CONST_FLOAT
        CRESWORD("", "double", CRK_LITERAL), // CTK_CONST_DOUBLE
        CRESWORD("", "character", CRK_LITERAL), // CTK_CONST_CHAR
        CRESWORD("", "string", CRK_EMPTY), // CTK_CONST_STRING
        CRESWORD("", "angle string", CRK_EMPTY), // CTK_ANGLE_STRING

        CRESWORD("", "pp-number", CRK_EMPTY), // CTK_PP_NUM
        CRESWORD("if", "#if", CRK_TOKEN),
        CRESWORD("ifdef", "#ifdef", CRK_TOKEN),
        CRESWORD("ifndef", "#ifndef", CRK_TOKEN),
        CRESWORD("elif", "#elif", CRK_TOKEN),
        CRESWORD("else", "#else", CRK_TOKEN),
        CRESWORD("endif", "#endif", CRK_TOKEN),
        CRESWORD("defined", "#define", CRK_TOKEN),

        CRESWORD("include", "#include", CRK_TOKEN),
        CRESWORD("pragma", "#pragma", CRK_TOKEN),
        CRESWORD("define", "#define", CRK_TOKEN),
        CRESWORD("undef", "#undef", CRK_TOKEN),
        CRESWORD("line", "#line", CRK_TOKEN),
        CRESWORD("error", "#error", CRK_TOKEN),
};

#undef CRESWORD
#undef CRK_TOKEN
#undef CRK_LITERAL
#undef CRK_EMPTY

#ifdef __cplusplus
}
#endif

#endif // !CRESWORDS_H
