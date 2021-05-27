#ifndef STRING_H
#define STRING_H

#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif

extern void* memset (void* ptr, int value, size_t num);
extern int memcmp(const void* p1, const void* p2, size_t num );
extern void* memcpy(void* dest, const void* src, size_t num);
extern size_t strlen(const char* s);
extern int strcmp(const char *str1, const char *str2);
extern int strncmp(const char* s1, const char* s2, size_t num);
extern char* strchr(const char* s, int c);
extern char* strcpy(char* dst, const char* src);
extern char* strncpy(char* dst, const char* src, size_t num);
extern char* strcat(char* dst, const char* src);
extern char* strstr(const char* s1, const char* s2);

typedef char* va_list;
extern int vsnprintf(char* s, size_t n, const char* format, va_list arg);

#endif
