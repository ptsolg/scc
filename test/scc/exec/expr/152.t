int main()
{
	double a = 1.0;
	long long b = 1;
	if ((a -= b) == 0.0)
		return 0;
	return 1;
}