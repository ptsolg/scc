int main()
{
	int a;
	int* b = &a;
	if ((b == 0) == 0)
		return 0;
	return 1;
}