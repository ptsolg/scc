struct {
	int a, b, c;
} d = { 1, 2, 3 };

int* b = &d.b;

int main()
{
	return !(*b == 2);
}