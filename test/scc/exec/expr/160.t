int main()
{
	int a[1];
	a[0] = 3;
	int b = 8;
	b >>= a[0];
	if (b == 1)
		return 0;
	return 1;
}