#include "scc/c/c-info.h"
#include "scc/c/c-reswords.h"
#include "scc/scl/char-info.h"

extern void cget_unescaped_string(char* buffer, const char* string, ssize len)
{
        for (ssize i = 0; i < len; i++)
        {
                int c = string[i];
                if (char_is_escape(c))
                {
                        *buffer++ = '\\';
                        *buffer++ = escape_to_char(c);
                }
                else
                        *buffer++ = c;
        }
        *buffer++ = '\0';
}

extern ssize cget_escaped_string(char* buffer, const char* string, ssize len)
{
        char* begin = buffer;
        for (ssize i = 0; i < len; i++)
        {
                int c = string[i];
                if (c == '\\')
                        *buffer++ = char_to_escape(string[++i]);
                else
                        *buffer++ = c;
        }
        *buffer++ = '\0';
        return buffer - begin;
}

#define ASSERT_ENUM_RANGE(V, MAX) S_ASSERT((V) > -1 && (V) < (MAX))

typedef struct
{
        const char* string;
} cbuiltin_type_info;

S_STATIC_ASSERT(12 == TBTK_SIZE, "cbuiltin_type_info_table needs update.");
static const cbuiltin_type_info cbuiltin_type_info_table[TBTK_SIZE] =
{
        { "" }, // TBTK_INVALID
        { "void" }, // TBTK_VOID
        { "char" }, // TBTK_INT8
        { "unsigned char" }, // TBTK_UINT8
        { "short" }, // TBTK_INT16
        { "unsigned short" }, // TBTK_UINT16
        { "int" }, // TBTK_INT32
        { "unsigned" }, // TBTK_UINT32
        { "long long" }, // TBTK_INT64
        { "unsigned long long" }, // TBTK_UINT64
        { "float" }, // TBTK_FLOAT
        { "double" }, // TBTK_DOUBLE
};

static inline const cbuiltin_type_info* cget_builtin_type_info(tree_builtin_type_kind k)
{
        ASSERT_ENUM_RANGE(k, TBTK_SIZE);
        return &cbuiltin_type_info_table[k];
}

extern const char* cget_builtin_type_string(tree_builtin_type_kind k)
{
        return cget_builtin_type_info(k)->string;
}

extern const char* cget_token_kind_string(ctoken_kind k)
{
        return ctoken_kind_to_string[k];
}

extern const cresword_info* cget_token_info(const ctoken* t)
{
        return cget_token_kind_info(ctoken_get_kind(t));
}

extern const cresword_info* cget_token_kind_info(ctoken_kind k)
{
        return &_creswords[k];
}

extern int cget_operator_precedence(const ctoken* self)
{
        return ctoken_is(self, CTK_QUESTION)
                ? CPL_CONDITIONAL
                : cget_binop_precedence(ctoken_to_binop(self));
}

typedef struct
{
        int precedence;
        const char* string;
} cbinop_info;

S_STATIC_ASSERT(31 == TBK_SIZE, "cbinop_info_table needs update.");
static const cbinop_info cbinop_info_table[TBK_SIZE] = 
{
        { CPL_UNKNOWN, "" }, // TBK_UNKNOWN
        { CPL_MULTIPLICATIVE, "*" }, // TBK_MUL
        { CPL_MULTIPLICATIVE, "/" }, // TBK_DIV
        { CPL_MULTIPLICATIVE, "%" }, // TBK_MOD
        { CPL_ADDITIVE, "+" }, // TBK_ADD
        { CPL_ADDITIVE, "-" }, // TBK_SUB
        { CPL_SHIFT, "<<" }, // TBK_SHL
        { CPL_SHIFT, ">>" }, // TBK_SHR
        { CPL_RELATIONAL, "<" }, // TBK_LE
        { CPL_RELATIONAL, ">" }, // TBK_GR
        { CPL_RELATIONAL, "<=" }, // TBK_LEQ
        { CPL_RELATIONAL, ">=" }, // TBK_GEQ
        { CPL_EQUALITY, "==" }, // TBK_EQ
        { CPL_EQUALITY, "!=" }, // TBK_NEQ
        { CPL_AND, "&" }, // TBK_AND
        { CPL_XOR, "^" }, // TBK_XOR
        { CPL_OR, "|" }, // TBK_OR
        { CPL_LOG_AND, "&&" }, // TBK_LOG_AND
        { CPL_LOG_OR, "||" }, // TBK_LOG_OR
        { CPL_ASSIGN, "=" }, // TBK_ASSIGN
        { CPL_ASSIGN, "+=" }, // TBK_ADD_ASSIGN
        { CPL_ASSIGN, "-=" }, // TBK_SUB_ASSIGN
        { CPL_ASSIGN, "*=" }, // TBK_MUL_ASSIGN
        { CPL_ASSIGN, "/=" }, // TBK_DIV_ASSIGN
        { CPL_ASSIGN, "%=" }, // TBK_MOD_ASSIGN
        { CPL_ASSIGN, "<<=" }, // TBK_SHL_ASSIGN
        { CPL_ASSIGN, ">>=" }, // TBK_SHR_ASSIGN
        { CPL_ASSIGN, "&=" }, // TBK_AND_ASSIGN
        { CPL_ASSIGN, "^=" }, // TBK_XOR_ASSIGN
        { CPL_ASSIGN, "|=" }, // TBK_OR_ASSIGN
        { CPL_COMMA, "," }, // TBK_COMMA
};

static inline const cbinop_info* cget_binop_info(tree_binop_kind k)
{
        ASSERT_ENUM_RANGE(k, TBK_SIZE);
        return &cbinop_info_table[k];
}

extern int cget_binop_precedence(tree_binop_kind k)
{
        return cget_binop_info(k)->precedence;
}

extern const char* cget_binop_string(tree_binop_kind k)
{
        return cget_binop_info(k)->string;
}

typedef struct
{
        const char* string;
} cunop_info;

S_STATIC_ASSERT(11 == TUK_SIZE, "cunop_info_table needs update.");
static const cunop_info cunop_info_table[TBK_SIZE] =
{
        { "" }, //TUK_UNKNOWN
        { "++" }, //TUK_POST_INC
        { "--" }, //TUK_POST_DEC
        { "++" }, //TUK_PRE_INC
        { "--" }, //TUK_PRE_DEC
        { "+" }, //TUK_PLUS
        { "-" }, //TUK_MINUS
        { "~" }, //TUK_NOT
        { "!" }, //TUK_LOG_NOT
        { "*" }, //TUK_DEREFERENCE
        { "&" }, //TUK_ADDRESS
};

static inline const cunop_info* cget_unop_info(tree_unop_kind k)
{
        ASSERT_ENUM_RANGE(k, TUK_SIZE);
        return &cunop_info_table[k];
}

extern const char* cget_unop_string(tree_unop_kind k)
{
        return cget_unop_info(k)->string;
}

extern const char* cget_decl_storage_class_string(tree_decl_storage_class sc)
{
        if (sc == TDSC_NONE || sc == TDSC_IMPL_EXTERN)
                return "";
        else if (sc == TDSC_EXTERN)
                return "extern";
        else if (sc == TDSC_REGISTER)
                return "register";
        else if (sc == TDSC_STATIC)
                return "static";
        return "";
}

extern void cqet_qual_string(tree_type_quals q, char* buf)
{
        *buf = '\0';
        if (q & TTQ_CONST)
                strcat(buf, "const ");
        if (q & TTQ_VOLATILE)
                strcat(buf, "volatile ");
        if (q & TTQ_RESTRICT)
                strcat(buf, "restrict ");
        
        if (buf[0])
                buf[strlen(buf) - 1] = '\0';
}

extern tree_binop_kind ctoken_to_binop(const ctoken* self)
{
        switch (ctoken_get_kind(self))
        {
                case CTK_STAR: return TBK_MUL;
                case CTK_SLASH: return TBK_DIV;
                case CTK_PERCENT: return TBK_MOD;
                case CTK_PLUS: return TBK_ADD;
                case CTK_MINUS: return TBK_SUB;
                case CTK_LE2: return TBK_SHL;
                case CTK_GR2: return TBK_SHR;
                case CTK_LE: return TBK_LE;
                case CTK_GR: return TBK_GR;
                case CTK_LEQ: return TBK_LEQ;
                case CTK_GREQ: return TBK_GEQ;
                case CTK_EQ2: return TBK_EQ;
                case CTK_EXCLAIM_EQ: return TBK_NEQ;
                case CTK_AMP: return TBK_AND;
                case CTK_CARET: return TBK_XOR;
                case CTK_VBAR: return TBK_OR;
                case CTK_AMP2: return TBK_LOG_AND;
                case CTK_VBAR2: return TBK_LOG_OR;
                case CTK_EQ: return TBK_ASSIGN;
                case CTK_PLUS_EQ: return TBK_ADD_ASSIGN;
                case CTK_MINUS_EQ: return TBK_SUB_ASSIGN;
                case CTK_STAR_EQ: return TBK_MUL_ASSIGN;
                case CTK_SLASH_EQ: return TBK_DIV_ASSIGN;
                case CTK_PERCENT_EQ: return TBK_MOD_ASSIGN;
                case CTK_LE2_EQ: return TBK_SHL_ASSIGN;
                case CTK_GR2_EQ: return TBK_SHR_ASSIGN;
                case CTK_AMP_EQ: return TBK_AND_ASSIGN;
                case CTK_CARET_EQ: return TBK_XOR_ASSIGN;
                case CTK_VBAR_EQ: return TBK_OR_ASSIGN;
                case CTK_COMMA: return TBK_COMMA;
                default: return TBK_UNKNOWN;
        }
}

extern tree_unop_kind ctoken_to_prefix_unary_operator(const ctoken* self)
{
        switch (ctoken_get_kind(self))
        {
                case CTK_AMP: return TUK_ADDRESS;
                case CTK_STAR: return TUK_DEREFERENCE;
                case CTK_PLUS: return TUK_PLUS;
                case CTK_MINUS: return TUK_MINUS;
                case CTK_TILDE: return TUK_NOT;
                case CTK_EXCLAIM: return TUK_LOG_NOT;
                case CTK_PLUS2: return TUK_PRE_INC;
                case CTK_MINUS2: return TUK_PRE_DEC;
                default: return TUK_UNKNOWN;
        }
}

extern bool ctoken_is_builtin_type_specifier(const ctoken* self)
{
        switch (ctoken_get_kind(self))
        {
                case CTK_VOID:
                case CTK_CHAR:
                case CTK_SHORT:
                case CTK_INT:
                case CTK_LONG:
                case CTK_FLOAT:
                case CTK_DOUBLE:
                case CTK_SIGNED:
                case CTK_UNSIGNED:
                        return true;

                default:
                        return false;
        }
}

extern bool ctoken_is_type_specifier(const ctoken* self)
{
        if (ctoken_is_builtin_type_specifier(self))
                return true;

        ctoken_kind k = ctoken_get_kind(self);
        return k == CTK_STRUCT || k == CTK_UNION || k == CTK_ENUM || k == CTK_ID;
}

extern tree_type_quals ctoken_to_type_qualifier(const ctoken* self)
{
        ctoken_kind k = ctoken_get_kind(self);
        if (k == CTK_CONST)
                return TTQ_CONST;
        else if (k == CTK_RESTRICT)
                return TTQ_RESTRICT;
        else if (k == CTK_VOLATILE)
                return TTQ_VOLATILE;

        return TTQ_UNQUALIFIED;
}

extern bool ctoken_is_type_qualifier(const ctoken* self)
{
        return ctoken_to_type_qualifier(self) != TTQ_UNQUALIFIED;
}

extern tree_decl_storage_class ctoken_to_decl_storage_class(const ctoken* self)
{
        ctoken_kind k = ctoken_get_kind(self);
        if (k == CTK_EXTERN)
                return TDSC_EXTERN;
        else if (k == CTK_STATIC)
                return TDSC_STATIC;
        else if (k == CTK_REGISTER)
                return TDSC_REGISTER;

        return TDSC_NONE;
}

extern bool ctoken_is_decl_storage_class(const ctoken* self)
{
        return ctoken_to_decl_storage_class(self) != TDSC_NONE;
}

extern bool ctoken_starts_declarator(const ctoken* self)
{
        ctoken_kind k = ctoken_get_kind(self);
        return k == CTK_STAR || k == CTK_ID || k == CTK_LBRACKET;
}

extern int cget_type_rank(const tree_type* t)
{
        return (int)tree_get_builtin_type_kind(tree_desugar_ctype(t));
}