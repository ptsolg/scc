struct {
	int a, b, c;
} d = { 1, 2, 3 };

int* b = (&d.b - 2) + 2;

int main()
{
	return !(*b == 2);
}