int main()
{
	int a[10];
	int* p = a + 1;

	*p-- = 1;
	*p = 2;

	if (a[1] != 1)
		return 1;
	if (a[0] != 2)
		return 1;
	return 0;
}