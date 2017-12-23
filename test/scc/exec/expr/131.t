int main()
{
	int* a;
	void* b;
	if ((int*)(b = a) == a)
		return 0;
	return 1;
}