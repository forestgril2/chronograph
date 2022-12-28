#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>

#include "Chronograph.h"
#define CHRONOGRAPH_TIME_LOGGING true


int main(int argc, char* argv[])
{

    Chronograph chrono("Profiler");
    for (auto i = 0; i < 1000000; ++i)
    {
       
    }
    chrono.log("Profiler");

    return 0;
}

