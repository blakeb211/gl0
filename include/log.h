#pragma once
#include <any>
#include <initializer_list>
#include <string>

// print arguments to log file and screen and add a newline
void LogPrintOneItem(const std::initializer_list<std::any>& il);


// This creates a lambda function and processes each arg of a vararg with it.
template<typename T, typename... Tail>
void LogPrintLn(T head, Tail... tail) {

	LogPrintOneItem({head});
	LogPrintOneItem({tail...});
  	LogPrintOneItem({ "\n" });
}

void LogErr(const std::string, const int, const std::string msg);

bool SetLogFile(std::string);

bool closeLog();
