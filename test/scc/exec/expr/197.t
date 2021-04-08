typedef struct
{
	int a;
	int b;
} Struct0;

typedef struct {
	int a;
	int b;
	Struct0 c;
} Struct1;

Struct1 a = { 1, 2, 3, 4 };

int main()
{
	return !(a.a == 1 && a.b == 2 && a.c.a == 3 && a.c.b == 4);
}