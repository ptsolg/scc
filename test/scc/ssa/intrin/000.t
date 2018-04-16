void test()
{
	volatile unsigned* ptr;
	unsigned expected;
	unsigned desired;
	int result = __atomic_cmpxchg_32_weak_seq_cst(ptr, expected, desired);
}