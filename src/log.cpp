#include "log.h"
#include <any>
#include <cstdio>
#include <initializer_list>
using namespace std;

//****************************************************************
// add handling for next type here
//****************************************************************

constexpr auto BUFFER_LENGTH = 512;

static FILE* fptr;

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

bool setLogFile(const char* fname) {
  if (strcmp(fname, "") == 0) {
    return false;
  }
  fptr = fopen(fname, "w+");
  if (fptr != NULL) {
    return true;
  } else {
    return false;
  }
}

template <class T>
void write_num_to_buffer(any val, char* buf) {
  int cCount = sprintf(buf, "%8.3Lf ", (long double)any_cast<T>(val));
  printf("%s", buf);
  fwrite(buf, sizeof(char), cCount, fptr);
}

void logPrintLn(initializer_list<any> il) {
  char buf[BUFFER_LENGTH] = "";
  int cCount = 0;
  for (auto& i : il) {
    const char* typeName = i.type().name();
    clearBuffer(buf, BUFFER_LENGTH);
    if (typeid(0.1f) == i.type()) {
      write_num_to_buffer<float>(i, buf);
      continue;
    }
    if (typeid(const char*) == i.type()) {
      cCount = sprintf(buf, "%s ", any_cast<const char*>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (typeid(char*) == i.type()) {
      cCount = sprintf(buf, "%s ", any_cast<char*>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (typeid(1) == i.type()) {
      write_num_to_buffer<int>(i, buf);
      continue;
    }
    if (typeid(1.0) == i.type()) {
      write_num_to_buffer<double>(i, buf);
      continue;
    }
    if (typeid(1u) == i.type()) {
      write_num_to_buffer<unsigned int>(i, buf);
      continue;
    }
    //****************************************************************
    // add handling for next type here
    //****************************************************************

    cCount =
        sprintf(buf, "ERROR: need to add new type to logPrint:<%s>", typeName);
    printf("%s", buf);
    fwrite(buf, sizeof(char), cCount, fptr);
  }
  // print a new line at the end regardless of what got printed
  cCount = sprintf(buf, "%s", "\n\0");
  printf("%s", buf);
  fwrite(buf, sizeof(char), cCount, fptr);
}

void clearBuffer(char* buf, int len) {
  int count = 0;
  while (count < len) {
    buf[count] = '\0';
    count++;
  }
}
