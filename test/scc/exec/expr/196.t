int a = 22;

struct {
	int* b;
} c = { &a };

int main()
{
	return !(*c.b == 22);
}