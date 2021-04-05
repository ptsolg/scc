typedef struct {
	int c;
	struct {
		int a;
	};
	int b;
} S;

S a = { 10, { .a = {10} } };