void test()
{
	unsigned* ptr;
	unsigned value;
	__atomic_xchg_32_seq_cst(ptr, value);
}