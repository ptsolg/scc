typedef void(*foo_t)();
typedef foo_t arr_t[1];

void test()
{
	arr_t arr;
	arr[0]; // foo_t
	arr[0](); // void
}