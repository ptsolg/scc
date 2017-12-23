int main()
{
	int a[10];
	int* p = a;

	*p++ = 1;
	*p = 2;

	if (a[0] != 1)
		return 1;
	if (a[1] != 2)
		return 1;
	return 0;
}