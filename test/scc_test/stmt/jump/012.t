int test()
{
	label1:
	{
		{
			{
				goto label1;
				goto label2;
			}
		}
	}
	label2:
	;
}