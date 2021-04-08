int a = 10;
int* b = &a;

int main()
{
	return !(*b == 10);
}