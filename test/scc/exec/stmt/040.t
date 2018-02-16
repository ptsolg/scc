int main()
{
	int a = 10;
	switch (a)
	{
		case 5:
		{
			a += 1;

			case 10:
				a += 1;

			if (a == 11)
				return 0;

			break;
		}
	}

	return 1;
}