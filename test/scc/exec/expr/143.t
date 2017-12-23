int main()
{
	int a[2];
	a[0] = 2;
	if ((a[0] %= 3) == 2)
		return 0;
	return 1;
}