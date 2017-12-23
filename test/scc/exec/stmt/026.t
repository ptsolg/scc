int main()
{
	int a = 0;
	do
	{
		a++;
		if (a)
			continue;
		a += 100;
	} while (a < 9);

	if (a == 9)
		return 0;
	return 1;
}