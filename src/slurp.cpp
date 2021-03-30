#define _CRT_SECURE_NO_WARNINGS
#include "..\include\slurp.h"
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include "log.h"

namespace slurp {
using namespace std;

std::stringstream get_file_contents(const char* filename) {
  FILE* fp = std::fopen(filename, "rb");
  if (fp) {
    string contents;
    fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    rewind(fp);
    fread(&contents[0], 1, contents.size(), fp);
    fclose(fp);
    return std::stringstream{contents};
  }
  // error string if fopen failed
  logPrintLn({__FILE__,
              "Method: get_file_contents(const char* filename) errno:", errno});
  return stringstream{""};
}

bool fileExists(string fname) {
  return true;
}

unique_ptr<vector<string>> split_file_to_lines() {}
// load_level parses a level file, loads models, and creates a level struct from
// it. should global startup produce a list of files in the level dir?

}  // namespace slurp