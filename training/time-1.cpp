//Author: Ugo Varetto
//std::chrono sample code
//use steady_clock for timing execution and system_clock for everything else
//on linux w/g++ all clocks have a period (resolution) of 1 ns
//NOTE: the latency of calling now() *seems* to be lower for the steady_clock
//implementation

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
    std::cout << "tick count type ";          
    //if tick count represenation behaves like a floating point number it means
    //that implicit conversions are allowed among durations and that division
    //of one value by another are acceptable 
    if(std::is_floating_point< system_clock::rep >::value) {
        std::cout << "behaves ";
    } else {
        std::cout << "does not behave "; 
    }
    std::cout << "like a floating point number" << std::endl;   

    //ratio is defined in the <ratio> header and represents any finite
    //rational number having a numerator and denominator defined as
    //compile-time constants of type intmax_t i.e. a type able to represent
    //values up to the biggest signed integer number, declared in <cstdint>  
    using Period = std::ratio< SecondsInAWeek(), INTMAX_C(1) >;          
    
    std::cout << "current period:  1 " 
              << RatioToString< Period >::Name()
              << std::endl;        
    
    //create a duration type with the clock's default arithmetic type
    //and a custom ratio = "seconds in a week" / 1
    //the duration 'count' method returns the number of rational numbers 
    //stored into the duration instance one tick = "seconds in a week" in
    //this case;      
    duration< system_clock::rep, Period > one_week(2);

    system_clock::time_point now = system_clock::now();
    system_clock::time_point last_week = now - one_week;
    std::time_t t;
    t = system_clock::to_time_t(now);
    std::cout << "now:             " << ctime(&t);
    t = system_clock::to_time_t(last_week);
    std::cout << "two weeks ago:   " << ctime(&t);
    const steady_clock::time_point t1 = steady_clock::now();
    const steady_clock::time_point t2 = steady_clock::now();
    std::cout << "delay (diff between two calls to now()): " 
              << (t2 - t1).count() << "ns" << std::endl;     
}

//------------------------------------------------------------------------------
void SteadyClock() {
    std::cout << "\nsteady_clock"
              << "\n------------\n"; 
    //check if clock is steady, most probably not, if not it should not be
    //used for timing code execution
    std::cout << "steady:          " << std::boolalpha 
              << steady_clock::is_steady << std::endl;
    //print out number of seconds in a period
    //clocks are defined by:
    // - arithmetic type used to represent a number of periods
    // - period: a rational type representing the number of seconds in a period
    // - steadiness: if the the clock can be adjusted or not          
    std::cout << "default period:  " 
              << steady_clock::period::num << ' '
              << RatioToString< Den< steady_clock::period >::type >::Name()
              << std::endl;
    std::cout << "tick count type ";          
    //if tick count represenation behaves like a floating point number it means
    //that implicit conversions are allowed among durations and that division
    //of one value by another are acceptable 
    if(std::is_floating_point< steady_clock::rep >::value) {
        std::cout << "behaves ";
    } else {
        std::cout << "does not behave "; 
    }
    std::cout << "like a floating point number" << std::endl;
    
    const steady_clock::time_point t1 = steady_clock::now();
    const steady_clock::time_point t2 = steady_clock::now();
    std::cout << "delay (diff between two calls to now()): " 
              << (t2 - t1).count() << "ns" << std::endl;           
}

//------------------------------------------------------------------------------
void HighResolutionClock() {
    std::cout << "\nhigh_resolution_clock"
              << "\n---------------------\n"; 
    //check if clock is steady, most probably not, if not it should not be
    //used for timing code execution
    std::cout << "steady:          " << std::boolalpha 
              << high_resolution_clock::is_steady << std::endl;
    //print out number of seconds in a period
    //clocks are defined by:
    // - arithmetic type used to represent a number of periods
    // - period: a rational type representing the number of seconds in a period
    // - steadiness: if the the clock can be adjusted or not          
    std::cout << "default period:  " 
              << high_resolution_clock::period::num << ' '
              << RatioToString< Den< high_resolution_clock::period >
                    ::type >::Name()
              << std::endl;
    std::cout << "tick count type ";          
    //if tick count represenation behaves like a floating point number it means
    //that implicit conversions are allowed among durations and that division
    //of one value by another are acceptable 
    if(std::is_floating_point< high_resolution_clock::rep >::value) {
        std::cout << "behaves ";
    } else {
        std::cout << "does not behave "; 
    }
    std::cout << "like a floating point number" << std::endl; 
    const high_resolution_clock::time_point t1 = high_resolution_clock::now();
    const high_resolution_clock::time_point t2 = high_resolution_clock::now();
    std::cout << "delay (diff between two calls to now()): "
              << (t2 - t1).count() << "ns" << std::endl;       
}

//------------------------------------------------------------------------------
void Duration() {
    std::cout << "\nduration"
              << "\n--------\n";
    using Week = std::ratio< SecondsInAWeek()  >;
    using Day  = std::ratio< SecondsInADay()   >;
    using Hour = std::ratio< SecondsInAnHour() >;
    using Minute = std::ratio< 60 >;
    using Second = std::ratio< 1 >;
    using Tick  = std::intmax_t;
    const duration< Tick, Week > weeks(4);
    const duration< Tick, Day  > days = weeks;
    const duration< Tick, Hour > hours = days;
    const duration< Tick, Minute > minutes = hours;
    const duration< Tick, Second > seconds = weeks;
    std::cout << weeks.count() << " weeks = " 
              << days.count()  << " days = " 
              << hours.count() << " hours = "
              << minutes.count() << " minutes = "
              << seconds.count() << " seconds\n"
              << std::endl;

    std::cout << "duration_cast:\n";
    const duration< float, Week > wf = 
        duration_cast< duration< float, Week > >(duration< Tick, Day >(15));
    std::cout << "15 days = " << wf.count() << " 'float' weeks\n";    
    const duration< Tick, Day > wd = 
        duration_cast< duration< Tick, Day > >(wf);
    std::cout <<  wf.count() << " 'float' weeks = " << wd.count() << " days\n";        
    const duration< Tick, Week > wi = 
        duration_cast< duration< Tick, Week > >(duration< Tick, Day >(15));
    std::cout << "15 days = " << wi.count() << " 'int' weeks\n";        
    const duration< Tick, Day > wdi = 
        duration_cast< duration< Tick, Day > >(wi);
    std::cout << wi.count() << " 'int' weeks = " << wdi.count() << " days\n";           

}

//------------------------------------------------------------------------------
int main(int, char**) {
    SystemClock();
    SteadyClock();
    HighResolutionClock();
    Duration();
    return 0;
}


// convenience typedefs
// typedef duration<signed integer type of at least 64 bits, nano> nanoseconds;
// typedef duration<signed integer type of at least 55 bits, micro> microseconds;
// typedef duration<signed integer type of at least 45 bits, milli> milliseconds;
// typedef duration<signed integer type of at least 35 bits > seconds;
// typedef duration<signed integer type of at least 29 bits, ratio< 60>> minutes;
// typedef duration<signed integer type of at least 23 bits, ratio<3600>> hours;
