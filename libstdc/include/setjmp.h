#ifndef SETJMP_H
#define SETJMP_H

#if _M32

typedef int _JBTYPE;
extern int _setjmp(_JBTYPE* buf);
#define setjmp _setjmp

#else

typedef struct _Aligned(16)
{
        unsigned long long _part[2];
} _JBTYPE;
extern int _setjmp(_JBTYPE* buf, void* fp);
#define setjmp(buf) _setjmp(buf, __frame_address(0))

#endif

typedef _JBTYPE jmp_buf[16];

extern void longjmp(jmp_buf buf, int val);

#endif
