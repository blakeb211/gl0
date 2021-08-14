#define _CRT_SECURE_NO_WARNINGS

//
#include "headers.h"

using namespace std;

namespace slurp {

  stringstream get_file_contents(const char* filename)
  {
    FILE* fp = fopen(filename, "rb");
    if (fp) {
      string contents;
      fseek(fp, 0, SEEK_END);
      contents.resize(std::ftell(fp));
      rewind(fp);
      fread(&contents[0], 1, contents.size(), fp);
      fclose(fp);
      return stringstream{ contents };
    }
    // error string if fopen failed
    logPrintLn(__FILE__,
      "Method: get_file_contents(const char* filename) errno:", errno);
    return stringstream{ "" };
  }

  stringstream get_file_contents(std::string filename)
  {
    return get_file_contents(filename.c_str());
  }

  // takes a and a name and returns true if it exists
  bool checkFileExist(const string path, const string fname, const string ext)
  {
    return filesystem::exists(path + fname + "." + ext);
  }

  // load_level parses a level file, loads models, and creates a level struct from
  // it. should global startup produce a list of files in the level dir?

} // namespace slurp
