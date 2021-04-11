union U
{
	int a;
	struct
	{
		int b;
		int c;
	} d;
} b = { .d = { 10 } };