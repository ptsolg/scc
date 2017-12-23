int main()
{
	float a = 1.0f;
	double b = 1.0;
	if ((a -= b) == 0.0f)
		return 0;
	return 1;
}