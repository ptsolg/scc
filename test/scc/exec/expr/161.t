int main()
{
	int a[1];
	a[0] = 8;
	int b[1];
	b[0] = 3;
	a[0] >>= b[0];
	if (a[0] == 1)
		return 0;
	return 1;
}