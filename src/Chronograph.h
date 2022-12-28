#pragma once

#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>
#include <utility>
#include <tuple>
#include <assert.h>

#define CHRONOGRAPH_TIME_LOGGING true

struct Chronograph
{
    using rep = unsigned long long;
    using period =  std::ratio<1, 260000000u>; // My machine is i7 9750

    // Define real time units
    using picoseconds = std::chrono::duration<unsigned long long, std::pico>;
    using nanoseconds = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds =  std::chrono::milliseconds;
    using duration = std::chrono::duration<rep, period>;
    using Cycle = std::chrono::duration<double, period>;
    // Define double-based unit of clock tick
    using TimePoint = std::chrono::time_point<Chronograph>;
    using ActionTime = std::tuple<std::string, TimePoint, microseconds>;

    Chronograph(const std::string&& measuredTimeAction, std::ostream* logStream = &std::cout, const bool isLoggingEnabled = true);
    ~Chronograph();

	static void setOutputFile(std::string filePath);
    static TimePoint now() noexcept;
    static void dumpTotalTimeActions();

    void setLoggingEnabled(bool isEnabled);
    void setLoggingDetailedOutput(bool isOutput);
    void start(const std::string& measuredTimeAction);
    void log(const std::string& measuredTimeAction = std::string());

private:

    static microseconds getMicroNow() noexcept;

    std::list<ActionTime> _measuredTimeActions; // Store each time interval for a given action
    static std::map<std::string, double> _totalActionTimes; // Store total time for each action
    bool _isLoggingEnabled = false;
    bool _isLoggingDetailedOutput = false;
    unsigned _nestingLevel = 0;
    static unsigned _runningChronographsCount;
    static unsigned _chronographSetup;
};
