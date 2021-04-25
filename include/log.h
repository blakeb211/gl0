#pragma once
#include <any>
#include <initializer_list>
#include <string>

// print arguments to log file and screen and add a newline
void logPrintLn(const std::initializer_list<std::any> &il);

void logErr(const std::string, const int, const std::string msg);

bool setLogFile(std::string);

bool closeLog();
