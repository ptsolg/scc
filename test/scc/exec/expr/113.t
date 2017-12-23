int main()
{
	int a;
	int* b = &a, *c = &a;
	if ((b != c) == 0)
		return 0;
	return 1;
}