int main()
{
	int a = 0;
	int b = 123124;
	a |= b;
	if (a == b)
		return 0;
	return 1;
}