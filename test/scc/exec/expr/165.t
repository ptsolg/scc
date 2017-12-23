int main()
{
	int a = 101;
	int b = a;
	a &= b;
	if (a == b)
		return 0;
	return 1;
}