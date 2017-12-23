int main()
{
	int a = 10;
	int b = 0;
	while(a--)
	{
		if (a == 1)
			continue;
		b++;
	}

	if (b == 9)
		return 0;
	return 1;
}