#include "time_wheel_scheduler.h"
#include "time_wheel.h"
#include "timer.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

static uint32_t s_inc_id = 1;

TimeWheelScheduler::TimeWheelScheduler(uint32_t timer_step_ms)
    : stop_(false), timer_step_ms_(timer_step_ms) {}

bool TimeWheelScheduler::Start() {
  if (timer_step_ms_ < 50) {
    return false;
  }
  if (time_wheels_.empty()) {
    return false;
  }
  thread_ = std::thread(std::bind(&TimeWheelScheduler::Run, this));
  return true;
}

void TimeWheelScheduler::Run() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(timer_step_ms_));

    std::lock_guard<std::mutex> lock(mutex_);
    if (stop_) {
      break;
    }
    TimeWheelPtr least_time_wheel = GetLeastTimeWheel(); // 推动时间轮要从小到大
    least_time_wheel->Increase();
    std::list<TimerPtr> slot = least_time_wheel->GetAndClearCurrentSlot();
    for (const TimerPtr &timer : slot) {
      auto it = cancel_timer_ids_.find(timer->id());
      if (it != cancel_timer_ids_.end()) { // 该timer已经被取消
        cancel_timer_ids_.erase(it);
        continue;
      }
      timer->Run();               // 执行task
      if (timer->repeated()) {    // timer的interval_ms>0时将会重复
        timer->UpdataeWhenTime(); // timer的when_ms向后偏移interval_ms
        GetGreatestTimeWheel()->AddTimer(timer); // 将偏移后的timer重新列装
      }
    }
  }
}

void TimeWheelScheduler::Stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }
  thread_.join();
}

TimeWheelPtr TimeWheelScheduler::GetGreatestTimeWheel() {
  if (time_wheels_.empty()) {
    return TimeWheelPtr();
  }
  return time_wheels_.front();
}

TimeWheelPtr TimeWheelScheduler::GetLeastTimeWheel() {
  if (time_wheels_.empty()) {
    return TimeWheelPtr();
  }
  return time_wheels_.back();
}

void TimeWheelScheduler::AppendTimeWheel(uint32_t scales,
                                         uint32_t scale_unit_ms,
                                         const std::string &name) {
  TimeWheelPtr time_wheel =
      std::make_shared<TimeWheel>(scales, scale_unit_ms, name);
  if (time_wheels_.empty()) {
    time_wheels_.push_back(time_wheel);
    return;
  }

  TimeWheelPtr greater_time_wheel = time_wheels_.back();
  greater_time_wheel->set_less_level_tw(time_wheel.get());
  time_wheel->set_greater_level_tw(greater_time_wheel.get());
  time_wheels_.push_back(time_wheel);
}

uint32_t TimeWheelScheduler::CreateTimerAt(int64_t when_ms,
                                           const TimerTask &task) {
  if (time_wheels_.empty()) {
    return 0;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  ++s_inc_id;
  GetGreatestTimeWheel()->AddTimer(
      std::make_shared<Timer>(s_inc_id, when_ms, 0, task));

  return s_inc_id;
}

uint32_t TimeWheelScheduler::CreateTimerAfter(int64_t delay_ms,
                                              const TimerTask &task) {
  int64_t when = GetNowTimestamp() + delay_ms;
  return CreateTimerAt(when, task);
}

uint32_t TimeWheelScheduler::CreateTimerEvery(int64_t interval_ms,
                                              const TimerTask &task) {
  if (time_wheels_.empty()) {
    return 0;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  ++s_inc_id;
  int64_t when = GetNowTimestamp() + interval_ms;
  // 为时间轮添加任务时要从大到小 逐级添加
  GetGreatestTimeWheel()->AddTimer(
      std::make_shared<Timer>(s_inc_id, when, interval_ms, task));

  return s_inc_id;
}

// timer采用手动取消
void TimeWheelScheduler::CancelTimer(uint32_t timer_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  cancel_timer_ids_.insert(timer_id);
}
