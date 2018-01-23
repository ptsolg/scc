typedef unsigned uint32_t;

const uint32_t array[10];

void foo(uint32_t v)
{
}

void test()
{
	foo(array[0] - 1);
}