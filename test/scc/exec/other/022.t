struct
{
	int a;
	char b[];
} c = { 10 };

int main()
{
	return c.a - 10;
}