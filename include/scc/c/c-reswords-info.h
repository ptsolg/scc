#ifndef C_RESWORDS_INFO_H
#define C_RESWORDS_INFO_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/common.h"
#include "c-token-kind.h"

static_assert(CTK_TOTAL_SIZE == 111, "Should update _c_resword_infos table.");

typedef struct _c_resword_info
{
        const char* string;
        const char* desription;
        const char* kind;
} c_resword_info;

#define C_RESWORD(s, d, k) {s, d, k}
#define CRK_TOKEN "token"
#define CRK_LITERAL "literal"
#define CRK_EMPTY ""

static const c_resword_info _c_resword_infos[CTK_TOTAL_SIZE] =
{
        C_RESWORD("", "unknown", CRK_TOKEN), // CTK_UNKNOWN
        C_RESWORD("", "end of file", CRK_EMPTY), // CTK_EOF
        C_RESWORD("", "end of line", CRK_EMPTY), // CTK_EOL
        C_RESWORD("", "end of macro", CRK_EMPTY), // CTK_EOM
        C_RESWORD("", "end of directive", CRK_EMPTY), // CTK_EOD
        C_RESWORD("", "white space", CRK_EMPTY), // CTK_WSPACE
        C_RESWORD("", "commentary", CRK_EMPTY), // CTK_COMMENT

        C_RESWORD("char", "'char'", CRK_TOKEN),
        C_RESWORD("short", "'short'", CRK_TOKEN),
        C_RESWORD("int", "'int'", CRK_TOKEN),
        C_RESWORD("long", "'long'", CRK_TOKEN),
        C_RESWORD("float", "'float'", CRK_TOKEN),
        C_RESWORD("double", "'double'", CRK_TOKEN),
        C_RESWORD("signed", "'signed'", CRK_TOKEN),
        C_RESWORD("unsigned", "'unsigned'", CRK_TOKEN),
        C_RESWORD("void", "'void'", CRK_TOKEN),

        C_RESWORD("struct", "'struct'", CRK_TOKEN),
        C_RESWORD("union", "'union'", CRK_TOKEN),
        C_RESWORD("enum", "'enum'", CRK_TOKEN),

        C_RESWORD("if", "'if'", CRK_TOKEN),
        C_RESWORD("else", "'else'", CRK_TOKEN),
        C_RESWORD("while", "'while'", CRK_TOKEN),
        C_RESWORD("for", "'for'", CRK_TOKEN),
        C_RESWORD("do", "'do'", CRK_TOKEN),
        C_RESWORD("break", "'break'", CRK_TOKEN),
        C_RESWORD("continue", "'continue'", CRK_TOKEN),
        C_RESWORD("return", "'return'", CRK_TOKEN),
        C_RESWORD("goto", "'goto'", CRK_TOKEN),
        C_RESWORD("switch", "'switch'", CRK_TOKEN),
        C_RESWORD("default", "'default'", CRK_TOKEN),
        C_RESWORD("case", "'case'", CRK_TOKEN),

        C_RESWORD("auto", "'auto'", CRK_TOKEN),
        C_RESWORD("register", "'register'", CRK_TOKEN),
        C_RESWORD("static", "'static'", CRK_TOKEN),
        C_RESWORD("extern", "'extern'", CRK_TOKEN),
        C_RESWORD("inline", "'inline'", CRK_TOKEN),
        C_RESWORD("const", "'const'", CRK_TOKEN),
        C_RESWORD("volatile", "'volatile'", CRK_TOKEN),
        C_RESWORD("restrict", "'restrict'", CRK_TOKEN),

        C_RESWORD("sizeof", "'sizeof'", CRK_TOKEN),
        C_RESWORD("typedef", "'typedef'", CRK_TOKEN),
        C_RESWORD("", "identifier", CRK_EMPTY), // CTK_ID

        C_RESWORD("{", "'{'", CRK_TOKEN),
        C_RESWORD("}", "'}'", CRK_TOKEN),
        C_RESWORD("[", "'['", CRK_TOKEN),
        C_RESWORD("]", "']'", CRK_TOKEN),
        C_RESWORD("(", "'('", CRK_TOKEN),
        C_RESWORD(")", "')'", CRK_TOKEN),
        C_RESWORD(".", "'.'", CRK_TOKEN),
        C_RESWORD("->", "'->'", CRK_TOKEN),
        C_RESWORD("<", "'<'", CRK_TOKEN),
        C_RESWORD(">", "'>'", CRK_TOKEN),
        C_RESWORD(">=", "'>='", CRK_TOKEN),
        C_RESWORD("<=", "'<='", CRK_TOKEN),
        C_RESWORD("==", "'=='", CRK_TOKEN),
        C_RESWORD("!=", "'!='", CRK_TOKEN),
        C_RESWORD("&&", "'&&'", CRK_TOKEN),
        C_RESWORD("||", "'||'", CRK_TOKEN),
        C_RESWORD("!", "'!'", CRK_TOKEN),
        C_RESWORD("~", "'~'", CRK_TOKEN),
        C_RESWORD("^", "'^'", CRK_TOKEN),
        C_RESWORD("&", "'&'", CRK_TOKEN),
        C_RESWORD("|", "'|'", CRK_TOKEN),
        C_RESWORD("+", "'+'", CRK_TOKEN),
        C_RESWORD("-", "'-'", CRK_TOKEN),
        C_RESWORD("*", "'*'", CRK_TOKEN),
        C_RESWORD("/", "'/'", CRK_TOKEN),
        C_RESWORD("%", "'%'", CRK_TOKEN),
        C_RESWORD("--", "'--'", CRK_TOKEN),
        C_RESWORD("++", "'++'", CRK_TOKEN),
        C_RESWORD("<<", "'<<'", CRK_TOKEN),
        C_RESWORD(">>", "'>>'", CRK_TOKEN),
        C_RESWORD("=", "'='", CRK_TOKEN),
        C_RESWORD("^=", "'^='", CRK_TOKEN),
        C_RESWORD("&=", "'&='", CRK_TOKEN),
        C_RESWORD("|=", "'|='", CRK_TOKEN),
        C_RESWORD("+=", "'+='", CRK_TOKEN),
        C_RESWORD("-=", "'-='", CRK_TOKEN),
        C_RESWORD("*=", "'*='", CRK_TOKEN),
        C_RESWORD("/=", "'/='", CRK_TOKEN),
        C_RESWORD("%=", "'%='", CRK_TOKEN),
        C_RESWORD("<<=", "'<<='", CRK_TOKEN),
        C_RESWORD(">>=", "'>>='", CRK_TOKEN),
        C_RESWORD(":", "':'", CRK_TOKEN),
        C_RESWORD(",", "','", CRK_TOKEN),
        C_RESWORD("?", "'?'", CRK_TOKEN),
        C_RESWORD(";", "';'", CRK_TOKEN),
        C_RESWORD("...", "'...'", CRK_TOKEN),
        C_RESWORD("#", "'#'", CRK_TOKEN),
        C_RESWORD("##", "'##'", CRK_TOKEN),

        C_RESWORD("", "integer", CRK_LITERAL), // CTK_CONST_INT
        C_RESWORD("", "float", CRK_LITERAL), // CTK_CONST_FLOAT
        C_RESWORD("", "double", CRK_LITERAL), // CTK_CONST_DOUBLE
        C_RESWORD("", "character", CRK_LITERAL), // CTK_CONST_CHAR
        C_RESWORD("", "string", CRK_EMPTY), // CTK_CONST_STRING
        C_RESWORD("", "angle string", CRK_EMPTY), // CTK_ANGLE_STRING

        C_RESWORD("", "pp-number", CRK_EMPTY), // CTK_PP_NUM
        C_RESWORD("if", "#if", CRK_TOKEN),
        C_RESWORD("ifdef", "#ifdef", CRK_TOKEN),
        C_RESWORD("ifndef", "#ifndef", CRK_TOKEN),
        C_RESWORD("elif", "#elif", CRK_TOKEN),
        C_RESWORD("else", "#else", CRK_TOKEN),
        C_RESWORD("endif", "#endif", CRK_TOKEN),

        C_RESWORD("include", "#include", CRK_TOKEN),
        C_RESWORD("define", "#define", CRK_TOKEN),
        C_RESWORD("undef", "#undef", CRK_TOKEN),
        C_RESWORD("line", "#line", CRK_TOKEN),
        C_RESWORD("error", "#error", CRK_TOKEN),
        C_RESWORD("pragma", "#pragma", CRK_TOKEN),

        C_RESWORD("_Atomic", "'_Atomic'", CRK_TOKEN),
        C_RESWORD("_Transaction_safe", "'_Transaction_safe'", CRK_TOKEN),
};

#undef C_RESWORD
#undef CRK_TOKEN
#undef CRK_LITERAL
#undef CRK_EMPTY

#ifdef __cplusplus
}
#endif

#endif
