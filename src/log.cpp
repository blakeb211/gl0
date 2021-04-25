#include "log.h"
//
#include <array>
#include <cstdio>
#include <fmt\core.h>
#include <fmt\format.h>
#include <fmt\os.h>
#include <string>

using namespace std;

static FILE *fptr;

// @TODO : replace cstyle file pointer with ???

bool closeLog() {
  if (fptr != NULL) {
    int result = fclose(fptr);
    if (result == 0) {
      return true;
    }
    return false;
  }
  return false;
}

bool setLogFile(std::string fname) {
  if (fname.empty())
    return false;

  fptr = fopen(fname.c_str(), "w+");
  if (fptr != NULL) {
    return true;
  } else {
    return false;
  }
}

template <class T> void write_num_to_buffer(any val) {
  string s = fmt::format("{:03.2f}  ", (long double)any_cast<T>(val));
  fmt::print(s);
  fwrite(s.data(), sizeof(char), s.length(), fptr);
}

void logPrintLn(const initializer_list<any> &il) {

  int cCount = 0;
  for (auto &i : il) {
    const char *typeName = i.type().name();
    if (typeid(0.1f) == i.type()) {
      write_num_to_buffer<float>(i);
      continue;
    }
    if (typeid(const char *) == i.type()) {
      string s = fmt::format("{} ", any_cast<const char *>(i));
      fmt::print(s);
      fwrite(s.data(), sizeof(char), s.length(), fptr);
      continue;
    }
    if (typeid(char *) == i.type()) {
      string s = fmt::format("{} ", any_cast<char *>(i));
      fmt::print(s);
      fwrite(s.data(), sizeof(char), s.length(), fptr);
      continue;
    }
    if (typeid(1) == i.type()) {
      write_num_to_buffer<int>(i);
      continue;
    }
    if (typeid(1.0) == i.type()) {
      write_num_to_buffer<double>(i);
      continue;
    }
    if (typeid(1u) == i.type()) {
      write_num_to_buffer<unsigned int>(i);
      continue;
    }
    if (typeid(string("a")) == i.type()) {
      string s = fmt::format("{} ", any_cast<string>(i));
      fmt::print(s);
      fwrite(s.data(), sizeof(char), s.length(), fptr);
      continue;
    }
    if (typeid(size_t) == i.type()) {
      write_num_to_buffer<size_t>(i);
      continue;
    }
    //****************************************************************
    // add handling for next type here
    //****************************************************************

    string s =
        fmt::format("ERROR: need to add new type to logPrint:<{}> ", typeName);
    fmt::print(s);
    fwrite(s.data(), sizeof(char), s.length(), fptr);
  }
  // print a new line at the end regardless of what got printed
  string s = fmt::format("\n");
  fmt::print(s);
  fwrite(s.data(), sizeof(char), s.length(), fptr);
}

void logErr(const char *fname, const int lineNum, const char *msg) {
  logPrintLn({"ERROR: file <", fname, "> line <", lineNum, "> ==", msg});
}
