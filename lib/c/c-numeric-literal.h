#ifndef C_NUMERIC_LITERAL_H
#define C_NUMERIC_LITERAL_H

#include "scc/tree/tree-common.h"

typedef struct _c_logger c_logger;

typedef enum
{
        CNLK_INVALID,
        CNLK_SP_FLOATING,
        CNLK_DP_FLOATING,
        CNLK_INTEGER,
} c_numeric_literal_kind;

typedef struct _c_numeric_literal
{
        c_numeric_literal_kind kind;
        const char* string;
        union
        {
                struct
                {
                        float value;
                } sp;

                struct
                {
                        double value;
                } dp;

                struct
                {
                        char* suffix;
                        uint64_t value;
                        bool is_signed;
                        int num_ls;
                        int num_us;
                        int radix;
                } integer;
        };
} c_numeric_literal;

extern bool c_parse_numeric_literal(
        const char* num,
        tree_location num_loc,
        c_numeric_literal* result,
        c_logger* logger);

#endif