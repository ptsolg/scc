int main()
{
	int a = 10;
	unsigned long long b = 10;
	if ((a -= b) == 0)
		return 0;
	return 1;
}