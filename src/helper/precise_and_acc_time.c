#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "precise_and_acc_time.h"

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <sys/time.h>
#endif

#ifdef _WIN32
const char is_accurate_time = 1;
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
const char is_accurate_time = 1;
#else
const char is_accurate_time = 0;
#endif

#include <limits.h>

const mstime_t mstime_t_min = LONG_MIN;
const mstime_t mstime_t_max = LONG_MAX;

mstime_t mstime() {
	#ifdef _WIN32
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
	
	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime( &system_time );
	SystemTimeToFileTime( &system_time, &file_time );
	time =  ((uint64_t)file_time.dwLowDateTime )      ;
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	return (mstime_t) ((time - EPOCH) / 10000L);
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return (mstime_t) tval.tv_sec * 1000L + (mstime_t) tval.tv_usec / 1000L;
	#else
	return (mstime_t) clock() * 1000 / CLOCKS_PER_SEC;
	#endif
}