int main()
{
	int a = 10;
	switch (a)
	{
		label:
			return 0;

		case 10:
			goto label;

		default:
			return 2;
	}

	return 1;
}