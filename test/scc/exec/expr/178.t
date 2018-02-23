int main()
{
	struct
	{
		int a;
	} b;
	b.a = 10;
	if (b.a != 10)
		return 1;
	return 0;
}