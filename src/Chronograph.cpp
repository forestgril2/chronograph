#include "Chronograph.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <utility>
#include <tuple>
#include <assert.h>

#include <stdint.h>
#include <boost/format.hpp>

using boost::format;
using boost::io::group;


#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static const unsigned logLineWidth = 30;

using rep = unsigned long long;
using period =  std::ratio<1, 190000000u>; // My machine is 1.6-1.9 GHz

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

static const std::string chronoLogsAction = "Chronograph logs";

static std::ofstream* _outputFile = nullptr;
static std::ostream* _logStream = &std::cout;

std::map<std::string, double> Chronograph::_totalActionTimes = {};
// std::map<std::string, double> Chronograph::_totalActionTimes = {{chronoLogsAction, 0.0}};
unsigned Chronograph::_runningChronographsCount = 0;

Chronograph::~Chronograph()
{
#ifdef CHRONOGRAPH_TIME_LOGGING
    // *_logStream << " DESTRUCT:  at                 " << getMicroNow().count() << " [us]" << std::endl;

    while(_measuredTimeActions.size() > 0)
    {// This will allow not to have remember to call log(), when getting out of a scope.
        *_logStream << " Chronograph::~Chronograph(): forced log. " << std::endl;
        log();
    }

    if (_runningChronographsCount > 0)
    {
        --_runningChronographsCount;
    }
    
    if (0 == _runningChronographsCount)
    {
        Chronograph::dumpTotalTimeActions();
        _totalActionTimes.clear();
    }
    // *_logStream << " DESTRUCTEND:  at " << getMicroNow().count() << " [us]" << std::endl;
#endif
}

Chronograph::Chronograph(const std::string&& measuredTimeAction, std::ostream* logStream, const bool isLoggingEnabled)
#ifdef CHRONOGRAPH_TIME_LOGGING
    : _isLoggingEnabled(isLoggingEnabled)
#endif
{
    _logStream = logStream;

#ifdef CHRONOGRAPH_TIME_LOGGING
    ++_runningChronographsCount;
    start(measuredTimeAction);
#endif
}

void Chronograph::setLoggingEnabled(bool isEnabled)
{
    _isLoggingEnabled = isEnabled;
}

void Chronograph::setLoggingDetailedOutput(bool isOutput)
{
    _isLoggingDetailedOutput = isOutput;
}

void Chronograph::setOutputFile(std::string filePath)
{
    if (_outputFile != nullptr)
    {
        _outputFile->close();
        delete _outputFile;
    }

    _outputFile = new std::ofstream(filePath);

    if (!_outputFile->is_open())
    {
        std::cout << " ### ERROR Chronograph:cannot open: " << filePath << std::endl;
        exit(-1);
    }
    std::cout << " ### INFO Chronograph: output file set: " << filePath << std::endl;
    _logStream = _outputFile;
}

Chronograph::TimePoint Chronograph::now() noexcept
{
//    unsigned lo, hi;
//    asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
//    return TimePoint(duration(static_cast<rep>(hi) << 32 | lo));
    return TimePoint(duration(__rdtsc()));
}

microseconds Chronograph::getMicroNow() noexcept
{
    return std::chrono::duration_cast<microseconds>(std::chrono::steady_clock::now().time_since_epoch());
}

template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
    return std::pair<B,A>(p.second, p.first);
}

template<typename A, typename B, template<class,class,class...> class M, class... Args>
std::multimap<B,A> flip_map(const M<A,B,Args...> &src)
{
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(),
                   std::inserter(dst, dst.begin()),
                   flip_pair<A,B>);
    return dst;
}

void Chronograph::dumpTotalTimeActions()
{
    // *_logStream << " DUMPstart:  at " << getMicroNow().count() << " [us]" << std::endl;
    // TODO: Realize it with bracket syntax, so that nested actions get printed as nested.
    *_logStream << " ### Chronograph::" <<  __FUNCTION__ << std::endl;

    const auto flippedTotalActionTimes = flip_map(_totalActionTimes);

    if (flippedTotalActionTimes.size() == 0)
        return;

    const double longestTime = (*flippedTotalActionTimes.rbegin()).first;
    double totalTime = 0.0;
    for(auto backIt = flippedTotalActionTimes.rbegin(); backIt != flippedTotalActionTimes.rend(); ++backIt)
    {
        totalTime += (*backIt).first;
        *_logStream << format("%10.1f %7.1f[ms] %1t %s \n") %  (((*backIt).first)*100.0/longestTime) %  (*backIt).first % (*backIt).second; 
    }
    *_logStream << " ### Chronograph::" <<  __FUNCTION__ << "(): sum of subtimes/longestTime: " << totalTime-longestTime << "/" << longestTime << std::endl;
    // *_logStream << " DUMPend:  at " << getMicroNow().count() << " [us]" << std::endl;
}

void Chronograph::start(const std::string& measuredTimeAction)
{
#ifdef CHRONOGRAPH_TIME_LOGGING
    if (!_isLoggingEnabled)
        return;

    ++_nestingLevel;

    if (_isLoggingDetailedOutput)
    {
        std::ostringstream out;
        // out << " ### Chronograph::" << __FUNCTION__ << "() for action:";
        // TODO: sort this out - how to align this elegantly.
        *_logStream << std::setw(logLineWidth) << out.str();
        for (unsigned i = 0; i < _nestingLevel + _runningChronographsCount -1; i++)
        {
            *_logStream << "    ";
        }
        *_logStream << measuredTimeAction << std::endl;
    }

    // *_logStream << " START : " << measuredTimeAction << " at " << getMicroNow().count() << " [us]" << std::endl;
    // Start timer after text streaming.
    _measuredTimeActions.push_back({measuredTimeAction, Chronograph::now(), getMicroNow()});
    // std::cout << "Chronograph::start() " << measuredTimeAction << std::endl;
#endif
}

void Chronograph::log(const std::string& measuredTimeAction)
{
#ifdef CHRONOGRAPH_TIME_LOGGING
    if (!_isLoggingEnabled || _measuredTimeActions.size() == 0)
        return;
    // *_logStream << " END   : " << measuredTimeAction << " at " << getMicroNow().count() << " [us]" << std::endl;

    // End timer before text streaming.
    const TimePoint t1 = Chronograph::now();
    const microseconds msT1 = getMicroNow();
    TimePoint t0;
    microseconds msT0;
    std::string measuredTimeActionFetched;

    if (measuredTimeAction.empty())
    {// Fetch the last action time point.
        measuredTimeActionFetched.swap(std::get<0>(_measuredTimeActions.back()));
        *_logStream << " ### WARNING Chronograph::" << __FUNCTION__ << "() automatically for action: " << measuredTimeActionFetched << std::endl;
        t0 = std::get<1>(_measuredTimeActions.back());
        msT0 = std::get<2>(_measuredTimeActions.back());
        _measuredTimeActions.pop_back();
    }
    else
    {// Look for the last matching string (serving as action "bracket" marker).
        auto revIt = _measuredTimeActions.rbegin();
        for ( ; revIt != _measuredTimeActions.rend(); ++revIt)
        {
            if (std::get<0>(*revIt) == measuredTimeAction)
            {
                measuredTimeActionFetched.swap(std::get<0>(*revIt));
                t0 = std::get<1>(*revIt);
                msT0 = std::get<2>(*revIt);
                break;
            }
        }

        // If no such, post a warning to log file and std::cout.
        if (revIt == _measuredTimeActions.rend())
        {
            const std::string warning = " ### WARNING Chronograph action: " + measuredTimeAction + " has not been found. ";
            std::cout << warning << std::endl;
            *_logStream << warning << std::endl;
            return;
        }

        _measuredTimeActions.erase(std::next(revIt).base());
    }


    // Get the clock ticks since restart for given action.
    const auto ticks = Chronograph::Cycle(t1 - t0);
    std::ostringstream out;
    if (_isLoggingDetailedOutput)
    {
        out << " ### Chronograph::" << __FUNCTION__ << "() for action:  ";
        // TODO: sort this out - how to align this elegangly.
        *_logStream << std::setw(logLineWidth) << out.str();
        for (unsigned i = 0; i < _nestingLevel + _runningChronographsCount -1; i++)
        {
            *_logStream << "    ";
            *_logStream << measuredTimeActionFetched << " # ms/ms(Chrono)/Mticks: "
                        << double((msT1-msT0).count())/1000.0 << " / "
                        << std::chrono::duration_cast<milliseconds>(ticks).count() << " / " << double(ticks.count())/1000000.0
                        << std::endl;
        }
    }

    _totalActionTimes[measuredTimeActionFetched] += double((msT1-msT0).count())/1000.0;
    // _totalActionTimes[chronoLogsAction] += double((getMicroNow()-msT1).count())/1000.0;

    --_nestingLevel;

    // *_logStream << " ENDlog: " << measuredTimeAction << " at " << getMicroNow().count() << " [us]" << std::endl;
#endif
}
