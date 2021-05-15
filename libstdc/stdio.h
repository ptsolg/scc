#ifndef STDIO_H
#define STDIO_H

#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif

extern int printf(const char* fmt, ...);
extern int sprintf(char* s, const char* fmt, ... );
extern int snprintf(char* s, size_t n, const char* fmt, ...);

typedef struct
{
	void* placeholder;
} FILE;

extern FILE* fopen(const char* file, const char* mode);
extern int fclose(FILE* file);
extern int fprintf(FILE* file, const char* format, ...);
extern size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream);
extern size_t fread(void* ptr, size_t size, size_t count, FILE* stream);
extern int fflush(FILE* file);

typedef char* va_list;
extern int vfprintf(FILE* stream, const char* fmt, va_list args);
extern int vprintf(const char* fmt, va_list args);

extern FILE* __acrt_iob_func(unsigned _x);

#define stdin (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

#endif
