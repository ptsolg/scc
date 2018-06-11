#ifndef _TM_WORD
#define _TM_WORD

#include <stddef.h> // size_t

typedef size_t _tm_word;

#define TM_MAX_WORD_MASK ((1 << sizeof(_tm_word)) - 1)

#endif