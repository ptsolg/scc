int main()
{
	int a[10];
	a[4] = 123;
	int* b = a + 5;
	int* c = b - 1;
	return *c != 123;
}