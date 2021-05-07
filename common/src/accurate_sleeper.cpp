#include "udaq/common/accurate_sleeper.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <timeapi.h>
#else
    #include <errno.h>
    #include <time.h>
#endif


udaq::common::AccurateSleeper::AccurateSleeper() {
#ifdef  _WIN32
    timeBeginPeriod(1);
#endif
}
udaq::common::AccurateSleeper::~AccurateSleeper(){
#ifdef  _WIN32
    timeEndPeriod(1);
#endif
}

void udaq::common::AccurateSleeper::sleep() {
#if defined(__linux__) || defined(__unix__) ||                                 \
    (defined(__APPLE__) && defined(__MACH__))
    struct timespec ts;
    ts.tv_sec = m_interval_ns / 1E9;
    ts.tv_nsec = m_interval_ns % 1000000000;

    /* Call until the time is passed, see nanosleep() manual) */
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
    //pselect(0, NULL, NULL, NULL, &ts, NULL);
#elif defined(_WIN32)
    auto milliseconds = m_interval_ns * 1E6;
    Sleep(milliseconds);
#endif
}
