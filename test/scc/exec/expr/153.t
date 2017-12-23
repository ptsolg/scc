int main()
{
	int _a = 10;
	int* a = &_a;
	*a -= 10ULL;
	if (*a == 0)
		return 0;
	return 1;
}