#include "c-builtin-type.h"

extern void c_builtin_type_init(c_builtin_type* self)
{
        self->kind = TBTK_INVALID;
        self->num_short = 0;
        self->num_long = 0;
        self->num_signed = 0;
        self->num_unsigned = 0;
}

extern bool c_builtin_type_set_kind(c_builtin_type* self, tree_builtin_type_kind kind)
{
        if (self->kind != TBTK_INVALID)
                return false; // kind is already set

        bool has_size_specs = self->num_long || self->num_short;
        bool has_sign_specs = self->num_signed || self->num_unsigned;
        bool has_specs = has_size_specs || has_sign_specs;
        if (kind == TBTK_VOID || kind == TBTK_FLOAT)
        {
                if (has_specs)
                        return false;
        }
        else if (kind == TBTK_INT32)
                ;
        else if (kind == TBTK_INT8)
        {
                if (has_size_specs)
                        return false;
        }
        else if (kind == TBTK_DOUBLE)
        {
                if (self->num_short || self->num_long > 2)
                        return false;
        }
        else
                UNREACHABLE(); // invalid kind

        self->kind = kind;
        return true;
}

extern tree_builtin_type_kind c_builtin_type_get_kind(const c_builtin_type* self)
{
        tree_builtin_type_kind kind = self->kind;
        if (kind == TBTK_VOID || kind == TBTK_FLOAT || kind == TBTK_DOUBLE)
                return kind;
        else if (kind == TBTK_INT8)
                return self->num_unsigned ? TBTK_UINT8 : TBTK_INT8;
        else if (kind == TBTK_INT32 || kind == TBTK_INVALID)
        {
                if (kind == TBTK_INVALID
                        && !self->num_short
                        && !self->num_long
                        && !self->num_signed
                        && !self->num_unsigned)
                {
                        return TBTK_INVALID; // kind was not set
                }

                if (self->num_long == 2)
                        return self->num_unsigned ? TBTK_UINT64 : TBTK_INT64;
                else if (self->num_short)
                        return self->num_unsigned ? TBTK_UINT16 : TBTK_INT16;

                return self->num_unsigned ? TBTK_UINT32 : TBTK_INT32;
        }

        UNREACHABLE();
        return TBTK_INVALID;
}

static inline bool c_builtin_type_can_have_size_or_sign_specifiers(tree_builtin_type_kind kind)
{
        return kind == TBTK_INVALID || kind == TBTK_INT8 || kind == TBTK_INT32;
}

extern bool c_builtin_type_add_signed_specifier(c_builtin_type* self)
{
        if (self->num_unsigned || !c_builtin_type_can_have_size_or_sign_specifiers(self->kind))
                return false;

        self->num_signed++;
        return true;
}

extern bool c_builtin_type_add_unsigned_specifier(c_builtin_type* self)
{
        if (self->num_signed || !c_builtin_type_can_have_size_or_sign_specifiers(self->kind))
                return false;

        self->num_unsigned++;
        return true;
}

extern bool c_builtin_type_add_short_specifier(c_builtin_type* self)
{
        if (self->num_long || self->num_short
                || !c_builtin_type_can_have_size_or_sign_specifiers(self->kind))
                return false;

        self->num_short++;
        return true;
}

extern bool c_builtin_type_add_long_specifier(c_builtin_type* self)
{
        if (self->num_short || self->num_long == 2
                || !c_builtin_type_can_have_size_or_sign_specifiers(self->kind))
                return false;

        self->num_long++;
        return true;
}