int main()
{
	int a[2];
	a[0] = 2;
	int b = 0;
	if ((b += a[0]) == 2)
		return 0;
	return 1;
}