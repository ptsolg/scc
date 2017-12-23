int main()
{
	int a[3];
	a[1] = 101;
	int* b = &a[0];
	b += 1;
	if (*b == 101)
		return 0;
	return 1;
}