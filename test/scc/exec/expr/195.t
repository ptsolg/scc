int a[3] = { [1] = 1, 2 };

int main()
{
	return !(a[0] == 0 && a[1] == 1 && a[2] == 2);
}