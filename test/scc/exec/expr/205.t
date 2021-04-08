typedef int arr[3];

arr a = { 1, 2, 3, [0] = 3, 2, 1};

int main()
{
	return !(a[0] == 3 && a[1] == 2 && a[2] == 1);
}