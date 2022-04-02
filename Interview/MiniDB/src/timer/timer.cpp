#include "timer.h"

Timer::Timer(uint32_t id, int64_t when_ms, int64_t interval_ms,
             const TimerTask &task)
    : id_(id), task_(task), when_ms_(when_ms), interval_ms_(interval_ms),
      repeated_(interval_ms_ > 0) {}

void Timer::Run() {
  if (task_) {
    task_();
  }
}
