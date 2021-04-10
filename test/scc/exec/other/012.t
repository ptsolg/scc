const char* t[] = { "a", "b", "c" };

int main()
{
	return !(t[0][0] == 'a' && t[1][0] == 'b' && t[2][0] == 'c');
}