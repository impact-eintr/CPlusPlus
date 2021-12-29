#ifndef TINYSTL_PROFILER_H_
#define TINYSTL_PROFILER_H_ 

#include <chrono>
#include <ratio>
#include <iostream>

class Profiler 
{
public:
    using SteadyClock   = std::chrono::steady_clock;
    using TimePoint     = SteadyClock::time_point;
    using SecDuration   = std::chrono::duration<double, std::ratio<1, 1>>;

private:
    static SecDuration  duration;
    static TimePoint    start;
    static TimePoint    finish;

public:
    static void Start()     // 开始计时
    {
        start = SteadyClock::now();
    }

    static void Finish()    // 结束计时
    {
        finish = SteadyClock::now();
        duration = std::chrono::duration_cast<SecDuration>(finish - start);
    }

    static void dumpDuration(std::ostream& os = std::cout)  // 打印时间
    {
        os << "Total time: " << duration.count() * 1000 << " (ms)" << std::endl;
    }
};

#endif
