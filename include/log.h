#pragma once
#include <any>
#include <initializer_list>
#include <string>

// print arguments to log file and screen and add a newline
void logPrintLn(const std::initializer_list<std::any>& il);

template<typename ... T>
void logPrintLn(T const& ... args) {
   
    ([&](auto const& element) {
   
   logPrintLn({element});

    }(args), ...);

	logPrintLn({"\n"});
}
 
void logErr(const std::string, const int, const std::string msg);

bool setLogFile(std::string);

bool closeLog();
