int main()
{
	int a;
	int* b = &a, *c = &a;
	if ((b == c) == 1)
		return 0;
	return 1;
}