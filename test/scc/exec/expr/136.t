int main()
{
	float a = 0.2f, b = 1.0f;
	if ((a *= b) == 0.2f)
		return 0;
	return 1;
}