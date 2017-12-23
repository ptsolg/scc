int main()
{
	int arr[10];
	int* it = arr;
	int* end = arr + 10;
	while(it != end)
		*it++ = 1;

	int i = 0;
	while (i < 10)
	{
		if (arr[i] != 1)
			return 1;
		i++;
	}

	return 0;
}