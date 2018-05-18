struct A
{
	union
	{
		void* a;
		void* b;
	};
};

void test()
{
	sizeof(struct A); // 4U
}