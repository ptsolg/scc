#include "numeric-literal.h"
#include "errors.h"
#include <stdlib.h> // strtoll, strtod, ...
#include <ctype.h> // toupper

static bool c_numeric_literal_is_float(const char* num)
{
        bool can_be_hex = false;
        while (*num)
        {
                int c = *num++;

                if (c == '.')
                        return true;

                if (toupper(c) == 'X')
                        can_be_hex = true;

                if (toupper(c) == 'E' && !can_be_hex)
                        return true;
        }
        return false;
}

static bool c_parse_floating_literal(
        const char* num,
        tree_location num_loc,
        c_numeric_literal* result,
        c_context* context)
{
        size_t len = strlen(num);
        char* suffix = NULL;
        bool sp = toupper(num[len - 1]) == 'F';

        if (sp)
                result->sp.value = strtof(num, &suffix);
        else
                result->dp.value = strtod(num, &suffix);

        size_t suffix_len = strlen(suffix);
        if ((sp && suffix_len > 1) || (!sp && suffix_len))
        {
                c_error_invalid_floating_literal(context, num_loc, num);
                return false;
        }

        result->kind = sp ? CNLK_SP_FLOATING : CNLK_DP_FLOATING;
        return true;
}

static bool c_parse_integer_literal_suffix(c_numeric_literal* result)
{
        const char* it = result->integer.suffix;
        while (*it)
        {
                int c = toupper(*it);
                if (c == 'L')
                        result->integer.num_ls++;
                else if (c == 'U')
                        result->integer.num_us++;
                else
                        return false;
                it++;
        }

        result->integer.is_signed = result->integer.num_us == 0;
        return result->integer.num_ls <= 2 && result->integer.num_us <= 1;
}

static bool c_parse_integer_literal(
        const char* num,
        tree_location num_loc,
        c_numeric_literal* result,
        c_context* context)
{
        result->integer.radix = num[0] == '0'
                ? toupper(num[1]) == 'X' ? 16 : 8
                : 10;

        result->integer.value = strtoull(num, &result->integer.suffix, result->integer.radix);
        if (!c_parse_integer_literal_suffix(result))
        {
                c_error_invalid_integer_literal(context, num_loc, num);
                return false;
        }

        result->kind = CNLK_INTEGER;
        return true;
}

extern bool c_parse_numeric_literal(
        const char* num,
        tree_location num_loc,
        c_numeric_literal* result,
        c_context* context)
{
        result->kind = CNLK_INVALID;
        result->string = num;
        result->integer.suffix = NULL;
        result->integer.value = 0;
        result->integer.is_signed = false;
        result->integer.num_ls = 0;
        result->integer.num_us = 0;
        result->integer.radix = 0;

        return c_numeric_literal_is_float(num)
                ? c_parse_floating_literal(num, num_loc, result, context)
                : c_parse_integer_literal(num, num_loc, result, context);
}