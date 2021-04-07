typedef struct _Expr
{
	struct _Expr* lhs, *rhs;
} Expr;

void test()
{
	Expr e = { 0, 0 };
}