union A 
{
	struct 
	{
		void* a;
	};
};

void test(int a, int b)
{
	union A a;
	a.a; // void*
}