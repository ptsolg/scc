int main()
{
	struct
	{
		int a, b, c;
	} d;

	d.c = 3;
	d.b = 2;
	d.a = 1;

	if (d.a != 1)
		return 1;
	if (d.b != 2)
		return 2;
	if (d.c != 3)
		return 3;
	return 0;
}