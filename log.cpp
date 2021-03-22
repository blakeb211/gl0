#include <log.h>

#include <stdio.h>

using namespace std;

void logPrint(const char * s, char * param) {
  printf(s);
  if (param != 0) {
    printf(param);
  }
  printf("\n");
}