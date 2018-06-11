#ifndef SETJMP_H
#define SETJMP_H

#if _M32
typedef int jmp_buf[16];
#else
typedef int jmp_buf[64];
#endif

#define setjmp _setjmp

extern int setjmp(int* buf);
extern void longjmp(int* buf, int val);

#endif