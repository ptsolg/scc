int main()
{
	int a[10];
	int* b = a;
	int* c = a + 3;
	int diff = c - b;
	return !(diff == 3);
}