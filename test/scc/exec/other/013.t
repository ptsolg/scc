int main()
{
	union
	{
		struct
		{
			int a, b;
		};
	} c = { .a=-10, .b=10 };

	return c.a + c.b;
}