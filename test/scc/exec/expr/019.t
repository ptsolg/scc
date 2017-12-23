int main()
{
	int a = 0;
	int* b = &a;
	int** c = &b;
	return c[0][0];
}