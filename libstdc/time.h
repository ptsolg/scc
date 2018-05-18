#ifndef TIME_H
#define TIME_H

typedef long clock_t;

#define CLOCKS_PER_SEC ((clock_t)1000)

extern clock_t clock();

#endif