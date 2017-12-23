int main()
{
	int a[10];
	a[5] = 123;
	int* p = 5 + a;
	return *p != 123;
}