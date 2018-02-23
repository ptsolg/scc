int main()
{
	struct
	{
		struct
		{
			int a;
		} b;
	} c;

	c.b.a = 11;
	if (c.b.a != 11)
		return 1;
	return 0;
}