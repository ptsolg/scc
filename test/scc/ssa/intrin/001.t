void test()
{
	unsigned* ptr;
	unsigned value;
	__atomic_add_fetch_32_seq_cst(ptr, value);
}