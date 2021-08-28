#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <array>
#include <cassert>
#include <cstdio>
#include <fmt\core.h>
#include <fmt\format.h>
#include <fmt\os.h>
#include <string>
#include <any>
//
// GLOBALS 
//
//
inline constexpr auto BUFFER_LEN = 512;
inline FILE* fptr;
inline std::array<char, BUFFER_LEN> buf;
/*******************************************************/

// print arguments to log file and screen and add a newline




template <class T> inline void write_to_screen_and_disk(const std::string fmt, std::any val) {
  const auto fmt_ = fmt;
  const auto arg_ = std::any_cast<T>(val);
  auto cnt_ = fmt::formatted_size(fmt_, arg_);
  auto _ = fmt::format_to_n(buf.begin(), cnt_, fmt_, arg_);
  fmt::print(fmt_, arg_);
  fwrite(buf.data(), sizeof(char), cnt_, fptr);
}

// This method has the LogPrintLn({ calling syntax and is only used internally 
// by log.h and log.cpp via the LogPrintLn template.
// It should not be used by any other files in case I want to remove it.
// 
// Instead should probably be customizing types with fmt::print
template<typename T>
inline void LogPrintOneItem(const T& item) {
  assert(fptr != nullptr);
  buf.fill('-');

  if (typeid(0.1f) == typeid(item)) {
    write_to_screen_and_disk<float>("{:<8.6f} ", item);
    return;
  }
  if (typeid(const char*) == typeid(std::decay<decltype(item)>::type)) {
    write_to_screen_and_disk<const char*>("{} ", item);
    return;
  }	
  if (typeid(char*) == typeid(item)) {
    write_to_screen_and_disk<char*>("{} ", item);
    return;
  }
  if (typeid(1) == typeid(item)) {
    write_to_screen_and_disk<int>("{:<8d} ", item);
    return;
  }
  if (typeid(1.0) == typeid(item)) {
    write_to_screen_and_disk<double>("{:<8.3f} ", item);
    return;
  }
  if (typeid(1u) == typeid(item)) {
    write_to_screen_and_disk<unsigned int>("{:<8d} ", item);
    return;
  }
  if (typeid(std::string("a")) == typeid(item)) {
    write_to_screen_and_disk<std::string>("{} ", item);
    return;
  }
  if (typeid(size_t) == typeid(item)) {
    write_to_screen_and_disk<size_t>("{:<8d} ", item);
    return;
  }
  //****************************************************************
  // add handling for next type here
  //****************************************************************
  { 
  #include <iostream>
	  std::cout << "Offending typeid:" << typeid(item).name() << std::endl;
  }
  write_to_screen_and_disk<const std::string>("ERROR: add type <{}> to LogPrintLn",
    typeid(item).name());
}


inline void LogPrintLn() {}
// This creates a lambda function and processes each arg of a vararg with it.
template<typename T, typename... Tail>
inline void LogPrintLn(T head, Tail... tail) {

  LogPrintOneItem(head);
  LogPrintLn(tail...);
  LogPrintOneItem("\n");
}



inline void LogErr(const std::string fname, const int lineNum, const std::string msg) {
  LogPrintLn("ERROR: file <", fname, "> line <", lineNum, "> ==", msg);
}

inline bool closeLog() {
  if (fptr != NULL) {
    int result = fclose(fptr);
    if (result == 0) {
      return true;
    }
    return false;
  }
  return false;
}

inline bool SetLogFile(std::string fname) {
  if (fname.empty()) {
    return false;
  }

  fptr = fopen(fname.c_str(), "w+");
  if (fptr != NULL) {
    return true;
  }
  else {
    return false;
  }
}
