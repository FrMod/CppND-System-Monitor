#include "format.h"
#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
    long  HH, MM, SS;
    HH = seconds / 3600;                // Reminder of seconds in an hour
    MM = (seconds - HH * 3600) / 60;    // Reminder of seconds in an minute
    SS = (seconds - HH * 3600 - MM *60) / 1;
    return std::to_string(HH) + ":" + std::to_string(MM) + ":" + std::to_string(SS);
}