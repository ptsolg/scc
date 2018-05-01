int main()
{
	// todo: test
	unsigned a;
	__atomic_xchg_32_seq_cst(&a, 0);
	return 0;
}