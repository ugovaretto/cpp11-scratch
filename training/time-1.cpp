//Author: Ugo Varetto
//std::chrono sample code

#include <chrono>
#include <ratio>
#include <cstdint>
#include <iostream>
#include <ctime>

using namespace std::chrono;

//------------------------------------------------------------------------------
constexpr std::intmax_t SecondsInAnHour() {
    return INTMAX_C(60 * 60);
}

constexpr std::intmax_t SecondsInADay() {
    return SecondsInAnHour() * INTMAX_C(24);
}

constexpr std::intmax_t SecondsInAWeek() {
    return SecondsInADay() * INTMAX_C(7);
}

//extract denominator
template < typename R >
struct Den {
    using type = std::ratio< INTMAX_C(1), R::den >;
};


//------------------------------------------------------------------------------
template < typename T >
struct RatioToString;

//standard specializations

template <>
struct RatioToString< std::pico > {
    static const char* Name() { return "ps"; } 
};

template <>
struct RatioToString< std::nano > {
    static const char* Name() { return "ns"; } 
};

template <>
struct RatioToString< std::micro > {
    static const char* Name() { return "us"; } 
};

template <>
struct RatioToString< std::milli > {
    static const char* Name() { return "ms"; } 
};

template <>
struct RatioToString< std::centi > {
    static const char* Name() { return "(1/100)s"; } 
};

template <>
struct RatioToString< std::deci > {
    static const char* Name() { return "(1/10)s"; } 
};

//custom specialization

template <>
struct RatioToString< std::ratio< INTMAX_C(1), INTMAX_C(1) > > {
    static const char* Name() { return "s"; }
};

template <>
struct RatioToString< std::ratio< SecondsInAnHour(), INTMAX_C(1) > > {
    static const char* Name() { return "hour(s)"; }
};

template <>
struct RatioToString< std::ratio< SecondsInADay(), INTMAX_C(1) > > {
    static const char* Name() { return "day(s)"; }
};

template <>
struct RatioToString< std::ratio< SecondsInAWeek(), INTMAX_C(1) > > {
    static const char* Name() { return "week(s)"; }
};

//------------------------------------------------------------------------------
void SystemClock() {
    std::cout << "\nsystem_clock"
              << "\n------------\n"; 
    //check if clock is steady, most probably not, if not it should not be
    //used for timing code execution
    std::cout << "steady:          " << std::boolalpha 
              << system_clock::is_steady << std::endl;
    //print out number of seconds in a period
    //clocks are defined by:
    // - arithmetic type used to represent a number of periods
    // - period: a rational type representing the number of seconds in a period
    // - steadiness: if the the clock can be adjusted or not          
    std::cout << "default period:  " 
              << system_clock::period::num << ' '
              << RatioToString< Den< system_clock::period >::type >::Name()
              << std::endl;
    
    using Period = std::ratio< SecondsInAWeek(), INTMAX_C(1) >;          
    
    std::cout << "current period:  1 " 
              << RatioToString< Period >::Name()
              << std::endl;        
    
    //create a duration type with the clock's default arithmetic type
    //and a custom ratio = "seconds in a week" / 1
    //the duration 'count' method returns the number of rational numbers 
    //stored into the duration instance one tick = "seconds in a week" in
    //this case;
    //ratio is defined in the <ratio> header and represents any finite
    //rational number having a numerator and denominator defined as
    //compile-time constants of type intmax_t i.e. a type able to represent
    //values up to the biggest signed integer number, declared in <cstdint>        
    duration< system_clock::rep, Period >
        one_week(2);
    system_clock::time_point now = system_clock::now();
    system_clock::time_point last_week = now - one_week;
    std::time_t t;
    t = system_clock::to_time_t(now);
    std::cout << "now:             " << ctime(&t);
    t = system_clock::to_time_t(last_week);
    std::cout << "two weeks ago:   " << ctime(&t);
}

//------------------------------------------------------------------------------
void SteadyClock() {

}

//------------------------------------------------------------------------------
void HighResolutionClock() {

}

//------------------------------------------------------------------------------
int main(int, char**) {
    SystemClock();
    SteadyClock();
    HighResolutionClock();
    return 0;
}


// convenience typedefs
// typedef duration<signed integer type of at least 64 bits, nano> nanoseconds;
// typedef duration<signed integer type of at least 55 bits, micro> microseconds;
// typedef duration<signed integer type of at least 45 bits, milli> milliseconds;
// typedef duration<signed integer type of at least 35 bits > seconds;
// typedef duration<signed integer type of at least 29 bits, ratio< 60>> minutes;
// typedef duration<signed integer type of at least 23 bits, ratio<3600>> hours;