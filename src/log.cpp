#define _CRT_SECURE_NO_WARNINGS
#include "log.h"
#include <any>
#include <initializer_list>
#include <stdio.h>

using namespace std;


const char *typeFloat = "float";
const char *typeConstCharPtr = "char const * __ptr64";
const char *typeCharPtr = "char * __ptr64";
const char *typeInt = "int";
const char *typeDouble = "double";
const char *typeUint = "unsigned int";
//****************************************************************
// add handling for next type here
//****************************************************************



static FILE *fptr;

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

bool setLogFile(const char *fname) {
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

void logPrintLn(initializer_list<any> il) {
  char buf[50] = "";
  int cCount = 0;
  for (auto &i : il) {
    const char *typeName = i.type().name();
      
    if (strcmp(typeName, typeFloat) == 0) {
      cCount = sprintf(buf, "%lf ", any_cast<float>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (strcmp(typeName, typeConstCharPtr) == 0) {
      cCount = sprintf(buf, "%s ", any_cast<const char *>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (strcmp(typeName, typeCharPtr) == 0) {
      cCount =  sprintf(buf, "%s ", any_cast<char *>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (strcmp(typeName, typeInt) == 0) {
      cCount = sprintf(buf, "%d ", any_cast<int>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (strcmp(typeName, typeDouble) == 0) {
      sprintf(buf, "%lf ", any_cast<double>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    if (strcmp(typeName, typeUint) == 0) {
      cCount = sprintf(buf, "%d ", any_cast<unsigned int>(i));
      printf("%s", buf);
      fwrite(buf, sizeof(char), cCount, fptr);
      continue;
    }
    //****************************************************************
    // add handling for next type here
    //****************************************************************



    cCount = sprintf(buf, "ERROR: need to add new type to logPrint:<%s>", typeName);
    printf("%s", buf);
    fwrite(buf, sizeof(char), cCount, fptr);
  }
  // print a new line at the end regardless of what got printed
  cCount = sprintf(buf, "%s", "\n");
  printf("%s", buf);
  fwrite(buf, sizeof(char), cCount, fptr);
}
