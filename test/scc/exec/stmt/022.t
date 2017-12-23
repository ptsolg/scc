int main()
{
	int a = 0;
	goto label;
	if (a)
	{
		if (a)
		{
			if (a)
			{
			label:
				;
				int b = 0;
				return b;
			}
		}
	}
	
	return 1;
}