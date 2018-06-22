#ifndef STDIO_H
#define STDIO_H

extern int printf(const char*, ...);

typedef struct
{
	void* placeholder;
} FILE;

extern FILE* fopen(const char* file, const char* mode);
extern int fclose(FILE* file);
extern int fprintf(FILE* file, const char* format, ...);

#endif