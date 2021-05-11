int main()
{
	typedef enum
	{
		VALUE = 10
	} Enum;
	Enum arr[] = { VALUE };
	return arr[0] - 10;
}