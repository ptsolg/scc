int main()
{
	struct
	{
		struct A* a;
	} b = { 0 };
	return (int)b.a;
}