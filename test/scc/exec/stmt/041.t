int main()
{
	int a = 10;
	switch (a)
	{
		case 10:
		{
			int b = 20;
			switch (b)
			{
				case 20:
					a = 100;
					b = 100;
					break;
			}
			
			if (a == 100)
				return 0;
		}
	}

	return 1;
}