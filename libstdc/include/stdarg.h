#ifndef STDARG_H
#define STDARG_H

typedef char* va_list;

#define va_start(list, param) __va_start((char*)&list)

#endif
