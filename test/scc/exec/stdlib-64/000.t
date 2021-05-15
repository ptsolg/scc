#include <setjmp.h>
#include <stdio.h>

int main()
{
	jmp_buf buf;
	if (setjmp(buf))
		printf("ok2");
	else
	{
		printf("ok1 ");
		longjmp(buf, 1);
	}
	return 0;
}