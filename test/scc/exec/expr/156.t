int main()
{
	char a = 1;
	unsigned long long b = 3;
	a <<= b;
	if (a == 8)
		return 0;
	return 1;
}