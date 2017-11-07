int test()
{
	for(;;)
		goto out;



	out:
		;
	int kind = 20;
	switch(kind)
	{
		case 10:
			return 20;

		case 15:
			for (int i = 0; i < 10; i++)
				kind++;
			return kind;

		case 20:
			return 41;
			
		default:
			break;
	}


	goto out;
	return 0;
}