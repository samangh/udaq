#pragma once

#include <chrono>
#include <atomic>

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
    void sleep();

  private:
    uint64_t m_interval_ns;
#ifdef _WIN32
    std::atomic<bool> m_time_period_set =false;
#endif
};

} // namespace udaq::common
