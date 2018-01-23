int main()
{
	char a = 1;
	unsigned char b = 1;
	short c = -1;
	unsigned short d = 1;
	int e = 1;
	unsigned f = -1;
	long long g = -10;
	unsigned long long h = 123;
	float j = -0.5f;
	double k = 0.5;
	void* l = (void*)123;

	if (a && b && c && d && e && f && g && h && j && k && l)
		return 0;
	return 1;
}