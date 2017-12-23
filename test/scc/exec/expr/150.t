int main()
{
	unsigned long long a = 10;
	char b = 10;
	if ((a -= b) == 0)
		return 0;
	return 1;
}