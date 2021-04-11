#include <udaq/common/gettimeofday.h>

#ifdef _WIN32
int gettimeofday(struct timeval * tp, struct timezone * tzp) {
       const unsigned __int64 epoch = 116444736000000000ull;
           FILETIME    file_time;
           SYSTEMTIME  system_time;
           ULARGE_INTEGER ularge;

           GetSystemTime(&system_time);
           SystemTimeToFileTime(&system_time, &file_time);
           ularge.LowPart = file_time.dwLowDateTime;
           ularge.HighPart = file_time.dwHighDateTime;

           tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
           tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

           return 0;
      }
#endif
