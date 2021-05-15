#ifndef TIME_H
#define TIME_H

typedef long clock_t;
typedef long long time_t;

#define CLOCKS_PER_SEC ((clock_t)1000)

struct tm
{
        int tm_sec;
        int tm_min;
        int tm_hour;
        int tm_mday;
        int tm_mon;
        int tm_year;
        int tm_wday;
        int tm_yday;
        int tm_isdst;
};

extern clock_t clock();
extern time_t time(time_t* arg);
extern struct tm* localtime(const time_t *timer);
#endif
