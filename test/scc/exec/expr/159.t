int main()
{
	unsigned a = 8;
	char b = 3;
	a >>= b;
	if (a == 1)
		return 0;
	return 1;
}