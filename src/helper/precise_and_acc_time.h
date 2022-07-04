#ifndef PRECISE_AND_ACC_TIME_H_INCLUDED
#define PRECISE_AND_ACC_TIME_H_INCLUDED

typedef long mstime_t;

extern const char is_accurate_time;
extern const mstime_t mstime_t_min;
extern const mstime_t mstime_t_max;

mstime_t mstime();

#endif // PRECISE_AND_ACC_TIME_H_INCLUDED