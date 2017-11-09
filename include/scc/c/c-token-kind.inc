CTOKEN(UNKNOWN),
CTOKEN(EOF),
CTOKEN(EOL),
CTOKEN(WSPACE),
CTOKEN(COMMENT),

// typespec
CKEYWORD(CHAR),
CKEYWORD(SHORT),
CKEYWORD(INT),
CKEYWORD(LONG),
CKEYWORD(FLOAT),
CKEYWORD(DOUBLE),
CKEYWORD(SIGNED),
CKEYWORD(UNSIGNED),
CKEYWORD(VOID),

CKEYWORD(STRUCT),
CKEYWORD(UNION),
CKEYWORD(ENUM),

// control flow
CKEYWORD(IF),
CKEYWORD(ELSE),
CKEYWORD(WHILE),
CKEYWORD(FOR),
CKEYWORD(DO),
CKEYWORD(BREAK),
CKEYWORD(CONTINUE),
CKEYWORD(RETURN),
CKEYWORD(GOTO),
CKEYWORD(SWITCH),
CKEYWORD(DEFAULT),
CKEYWORD(CASE),

// modifiers
CKEYWORD(AUTO),
CKEYWORD(REGISTER),
CKEYWORD(STATIC),
CKEYWORD(EXTERN),
CKEYWORD(INLINE),
CKEYWORD(CONST),
CKEYWORD(VOLATILE),
CKEYWORD(RESTRICT),

// other
CKEYWORD(SIZEOF),
CKEYWORD(TYPEDEF),
CKEYWORD(ID),

// punctuators
CPUNCTUATOR(LBRACE),     // {
CPUNCTUATOR(RBRACE),     // }
CPUNCTUATOR(LSBRACKET),  // [
CPUNCTUATOR(RSBRACKET),  // ]
CPUNCTUATOR(LBRACKET),   // (
CPUNCTUATOR(RBRACKET),   // )
CPUNCTUATOR(DOT),        // .
CPUNCTUATOR(ARROW),      // ->
CPUNCTUATOR(LE),         // <
CPUNCTUATOR(GR),         // >
CPUNCTUATOR(GREQ),       // >=
CPUNCTUATOR(LEQ),        // <=
CPUNCTUATOR(EQ2),        // ==
CPUNCTUATOR(EXCLAIM_EQ), // !=
CPUNCTUATOR(AMP2),       // &&
CPUNCTUATOR(VBAR2),      // ||
CPUNCTUATOR(EXCLAIM),    // !
CPUNCTUATOR(TILDE),      // ~
CPUNCTUATOR(CARET),      // ^
CPUNCTUATOR(AMP),        // &
CPUNCTUATOR(VBAR),       // |
CPUNCTUATOR(PLUS),       // +
CPUNCTUATOR(MINUS),      // -
CPUNCTUATOR(STAR),       // *
CPUNCTUATOR(SLASH),      // /
CPUNCTUATOR(PERCENT),    // %
CPUNCTUATOR(MINUS2),     // --
CPUNCTUATOR(PLUS2),      // ++
CPUNCTUATOR(LE2),        // <<
CPUNCTUATOR(GR2),        // >>
CPUNCTUATOR(EQ),         // =
CPUNCTUATOR(CARET_EQ),   // ^=
CPUNCTUATOR(AMP_EQ),     // &=
CPUNCTUATOR(VBAR_EQ),    // |=
CPUNCTUATOR(PLUS_EQ),    // +=
CPUNCTUATOR(MINUS_EQ),   // -=
CPUNCTUATOR(STAR_EQ),    // *=
CPUNCTUATOR(SLASH_EQ),   // /=
CPUNCTUATOR(PERCENT_EQ), // %=
CPUNCTUATOR(LE2_EQ),     // <<=
CPUNCTUATOR(GR2_EQ),     // >>=
CPUNCTUATOR(COLON),      // :
CPUNCTUATOR(COMMA),      // ,
CPUNCTUATOR(QUESTION),   // ?
CPUNCTUATOR(SEMICOLON),  // ;
CPUNCTUATOR(ELLIPSIS),   // ...
CPUNCTUATOR(HASH),       // #
CPUNCTUATOR(HASH2),      // ##

// constants
CTOKEN(CONST_INT),     
CTOKEN(CONST_FLOAT),
CTOKEN(CONST_DOUBLE), 
CTOKEN(CONST_CHAR),   // 'a'
CTOKEN(CONST_STRING), // "string"
CTOKEN(ANGLE_STRING), // <string>

// preprocessor
CPP_KEYWORD(NUM),
CPP_KEYWORD(IF),
CPP_KEYWORD(IFDEF),
CPP_KEYWORD(IFNDEF),
CPP_KEYWORD(ELIF),
CPP_KEYWORD(ELSE),
CPP_KEYWORD(ENDIF),
CPP_KEYWORD(DEFINED),

CPP_KEYWORD(INCLUDE),
CPP_KEYWORD(DEFINE),
CPP_KEYWORD(UNDEF),
CPP_KEYWORD(LINE),
CPP_KEYWORD(ERROR),
CPP_KEYWORD(PRAGMA),

CTOKEN(TOTAL_SIZE),