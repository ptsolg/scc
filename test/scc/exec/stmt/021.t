int main()
{
	int i = 0;
loop:
	if (i == 10)
		goto out;

	i++;
	goto loop;

out:
	return 0;
}