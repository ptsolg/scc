int main()
{
	unsigned a = 0;
	unsigned b = 123124;
	a |= b;
	if (a == b)
		return 0;
	return 1;
}