#define _CRT_SECURE_NO_WARNINGS
#include "..\include\slurp.h"
#include <errno.h>
#include <log.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

namespace slurp {

std::stringstream get_file_contents(const char* filename) {
  std::FILE* fp = std::fopen(filename, "rb");
  if (fp) {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return std::stringstream{contents};
  }
  // error string if fopen failed
  logPrintLn({__FILE__,
              "Method: get_file_contents(const char* filename) errno:", errno});
  return std::stringstream{""};
}

}  // namespace slurp