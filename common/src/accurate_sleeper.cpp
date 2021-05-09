#include "udaq/common/accurate_sleeper.h"

#include <stdexcept>
#include <string.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <timeapi.h>
#else
    #include <pthread.h>
    #include <errno.h>
    #include <time.h>
#endif

#ifdef __linux__
    #include <sched.h>
#endif

udaq::common::AccurateSleeper::AccurateSleeper() {}
udaq::common::AccurateSleeper::~AccurateSleeper(){
    disable_realtime();
}

void udaq::common::AccurateSleeper::set_interval(uint32_t interval_ns) {
    enable_realtime();
    m_interval_ns = interval_ns;
}

void udaq::common::AccurateSleeper::enable_realtime()
{
    if (!m_realtime_enabled)
    {
#if defined(__linux__) || defined(__unix__) ||                                 \
    (defined(__APPLE__) && defined(__MACH__))
        /* Store the thread, so tha later on we restore the schedulign policy for teh same thread */
        m_thread = pthread_self();

        /* Get current policy for later restoration */
        m_previous_policy = std::make_unique<int>();
        m_previous_param = std::make_unique<struct sched_param>();
        if (pthread_getschedparam(m_thread, m_previous_policy.get(), m_previous_param.get()) !=0 )
                throw std::runtime_error(std::string("Unable to get the current thread scheduling parameters, ") + strerror(errno));

        /* Set realtime priority to minimum */
        struct sched_param param;
#ifdef __linux__
        param.sched_priority= sched_get_priority_min(SCHED_FIFO);
#else
        param.sched_priority= PTHREAD_MIN_PRIORITY;
#endif

        /* Set the scheduler using POSIX thread */
        if (pthread_setschedparam(m_thread, SCHED_FIFO, &param) !=0)
            throw std::runtime_error(std::string("Unable to enable real-time scheduling, ") + strerror(errno));
#elif defined(_WIN32)
        timeBeginPeriod(1);
        m_realtime_enabled=true;
#endif

        m_realtime_enabled=true;
    }
}

void udaq::common::AccurateSleeper::disable_realtime()
{
    if (m_realtime_enabled)
    {
#if defined(__linux__) || defined(__unix__) ||                                 \
    (defined(__APPLE__) && defined(__MACH__))
        if (pthread_setschedparam(m_thread, *(m_previous_policy.get()), m_previous_param.get()) !=0)
            throw std::runtime_error(std::string("Unable to disable real-time scheduling, ") + strerror(errno));
#elif defined(_WIN32)
        if (m_time_period_set)
            timeEndPeriod(1);
#endif

        m_realtime_enabled=false;
    }
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
    auto milliseconds = (DWORD)m_interval_ns / 1000000;
    Sleep(milliseconds);
#endif
}
