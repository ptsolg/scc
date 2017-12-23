int main()
{
	int a = 0;
again:
	if (a == 100)
		return 0;

	a = 100;
	goto again;
}