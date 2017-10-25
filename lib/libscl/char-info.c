#include "char-info.h"

const int _ascii_char_info[256] =
{
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        //           tab          \n
        ACK_UNKNOWN, ACK_WSPACE,  ACK_NEWLINE, ACK_UNKNOWN,
        //           \r
        ACK_UNKNOWN, ACK_NEWLINE, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        // space     !            "            #
        ACK_WSPACE,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,
        // $         %            &            '
        ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,
        // (         )            *            +
        ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,
        // ,         -            .            /
        ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,
        // 0         1            2            3
        ACK_DIGIT,   ACK_DIGIT,   ACK_DIGIT,   ACK_DIGIT,
        // 4         5            6            7
        ACK_DIGIT,   ACK_DIGIT,   ACK_DIGIT,   ACK_DIGIT,
        // 8         9            :            ;
        ACK_DIGIT,   ACK_DIGIT,   ACK_SYMBOL,  ACK_SYMBOL,
        // <         =            >            ?
        ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,
        // @         A            B            C ...
        ACK_SYMBOL,  ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        // ... X     Y            Z            [
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_SYMBOL,
        // \         ]            ^            _
        ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_ALPHA,
        // `         a            b            c...
        ACK_SYMBOL,  ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,
        // ... x     y            z            {
        ACK_ALPHA,   ACK_ALPHA,   ACK_ALPHA,   ACK_SYMBOL,
        // |         }            ~
        ACK_SYMBOL,  ACK_SYMBOL,  ACK_SYMBOL,  ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
        ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN, ACK_UNKNOWN,
};