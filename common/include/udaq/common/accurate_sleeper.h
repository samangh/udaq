#pragma once

#include <chrono>
#include <atomic>
#include <memory>

#ifndef _WIN32
  #include <pthread.h>
#endif

namespace udaq::common {

class AccurateSleeper {
  public:
    AccurateSleeper();
    ~AccurateSleeper();
    template <class _reprsenetation, class _value>
    void set_interval(
        const std::chrono::duration<_reprsenetation, _value> &duration) {
        if (duration < std::chrono::duration<_reprsenetation, _value>::max()) {
            auto a =
                std::chrono::duration_cast<std::chrono::nanoseconds>(duration)
                    .count();
            set_interval(a);
        }
    }
    void set_interval(uint32_t interval_ns);
    void enable_realtime();
    void disable_realtime();
    void sleep();

  private:
    uint64_t m_interval_ns;
    std::atomic<bool> m_realtime_enabled =false;
#ifndef _WIN32
    pthread_t m_thread;
    std::unique_ptr<int> m_previous_policy;
    std::unique_ptr<struct sched_param> m_previous_param;
#endif

};

} // namespace udaq::common
