#define A(a, b) a ## b
#define B (
#define C )
#define D(a, b) A B a, b C

D(1, 2)