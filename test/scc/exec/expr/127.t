int main()
{
	unsigned a = 123, b = -1;
	if ((a ^ b) == 4294967172)
		return 0;
	return 1;
}