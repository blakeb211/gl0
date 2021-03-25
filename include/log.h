#pragma once
#include <initializer_list>
#include <any>

// print arguments to log file and screen and add a newline
void logPrintLn(std::initializer_list<std::any> il);

void clearBuffer(char* buf, int len);

bool setLogFile(const char * fname);

bool closeLog();
