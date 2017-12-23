int main()
{
	int a[2];
	a[0] = 101;
	int* p = &a[1];
	p -= 1;
	if (*p == 101)
		return 0;
	return 1;
}