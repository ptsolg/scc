struct {
	int a, b;
} c = { 1, 2 };

int main()
{
	return !(c.a == 1 && c.b == 2);
}