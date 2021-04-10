#define _X(A, B) A ## B
#define X(A, B) _X(A, B)
#define HTAB htab
#define HTAB_ENTRY X(HTAB, _entry)
HTAB_ENTRY