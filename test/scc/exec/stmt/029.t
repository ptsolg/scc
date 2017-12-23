int main()
{
	int i, j;
	for (i = 0, j = 0; i < 10, j < 5; i++, j++)
		;

	if (i != 5)
		return 1;
	if (j != 5)
		return 1;

	return 0;
}