typedef struct _Type
{
	int kind;
} Type;

typedef struct _Expr
{
	Type t;
} Expr;

void test()
{
	Expr e = { 0 };
}