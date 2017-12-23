int main()
{
	int a = 10;
	while ((a = a - 1))
	{
		if (a == 5)
			break;
		else
			continue;
	}

	if (a == 5)
		return 0;
	return a;
}