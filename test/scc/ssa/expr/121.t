typedef struct _Binop
{
	int opcode;
} Binop;

typedef struct _Unop
{
	int opcode;
} Unop;

typedef struct _Expr
{
	union {
		Binop binop;
		Unop unop;
	};
} Expr;

void test()
{
	Expr e = { 0 };
}