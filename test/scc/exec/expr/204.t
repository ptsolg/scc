struct {
	int a, b, c;
} d = { 1, 2, 3, .a = 3, 2, 1 };

int main()
{
	return !(d.a == 3 && d.b == 2 && d.c == 1);
}