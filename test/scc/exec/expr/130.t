int main()
{
	int* a;
	void* b;
	if ((a = b) == (int*)b)
		return 0;
	return 1;
}