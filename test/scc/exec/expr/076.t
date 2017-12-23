int main()
{
	int a;
	int* b = &a, *c = &a - 1;
	if (b > c == 1)
		return 0;
	return 1;
}