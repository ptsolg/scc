int main()
{
	int a[10];
	a[2] = 123;
	int* b = &a[1] + 1;
	return *b != 123;
}