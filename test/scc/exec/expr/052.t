int main()
{
	int a[10];
	a[5] = 123;
	int* p = a + 5;
	return *p != 123;
}