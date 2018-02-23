int main()
{
	union
	{
		int val;
		struct
		{
			short lower;
			short upper;
		};
	} u;

	u.lower = 2;
	u.upper = 2;

	if (u.val != 131074)
		return 1;
	return 0;
}