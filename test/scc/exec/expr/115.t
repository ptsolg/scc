int main()
{
	int a;
	int* b = &a;
	if ((0 != b) == 1)
		return 0;
	return 1;
}