int main()
{
	unsigned a = 9, b = 9;
	if ((a *= b) == 81)
		return 0;
	return 1;
}