int main()
{
	unsigned long long a = 0;
	char b = 113;
	a |= b;
	if (a == b)
		return 0;
	return 1;
}