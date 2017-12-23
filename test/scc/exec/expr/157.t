int main()
{
	unsigned a = 1;
	int b = 3;
	a <<= b;
	if (a == 8)
		return 0;
	return 1;
}