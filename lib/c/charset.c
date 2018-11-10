#include "scc/c/charset.h"

const int c_char_info_table[256] =
{
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        //              \t              \n              
        CCK_UNKNOWN,    CCK_SPACE,      CCK_UNKNOWN,    CCK_UNKNOWN,
        //              \r                              
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        // space        !               "               #
        CCK_SPACE,      CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR,
        // $            %               &               '
        CCK_UNKNOWN,    CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR,
        // (            )               *               +
        CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR,
        // ,            -               .               /
        CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR,
        // 0            1               2               3
        CCK_DIGIT,      CCK_DIGIT,      CCK_DIGIT,      CCK_DIGIT,
        // 4            5               6               7
        CCK_DIGIT,      CCK_DIGIT,      CCK_DIGIT,      CCK_DIGIT,
        // 8            9               :               ;
        CCK_DIGIT,      CCK_DIGIT,      CCK_PUNCTUATOR, CCK_PUNCTUATOR,
        // <            =               >               ?
        CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR,
        // @            A               B               C
        CCK_UNKNOWN,    CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        // X            Y               Z               [
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_PUNCTUATOR,
        // \            ]               ^               _
        CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_LETTER,
        // `            a               b               c
        CCK_UNKNOWN,    CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_LETTER,
        // x            y               z               {
        CCK_LETTER,     CCK_LETTER,     CCK_LETTER,     CCK_PUNCTUATOR,
        // |            }               ~
        CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_PUNCTUATOR, CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
        CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,    CCK_UNKNOWN,
};