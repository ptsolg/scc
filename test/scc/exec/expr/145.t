int main()
{
	unsigned a = -1;
	if ((a += 0) == -1)
		return 0;
	return 1;
}