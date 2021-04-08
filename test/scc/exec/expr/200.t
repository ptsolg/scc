int a[3] = { 1, 2, 3};
int* b = a + 1;

int main()
{
	return !(*b == 2);
}