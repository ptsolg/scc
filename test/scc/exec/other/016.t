int main()
{
	union
	{
		int a;
		float b;
	} c = { .b = 10 };
	return c.b - 10;
}