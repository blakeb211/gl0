#define _CRT_SECURE_NO_WARNINGS

//
#include "headers.h"
#include "log.h"
using namespace std;

namespace slurp {

  stringstream GetFileContents(const char* filename)
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
    LogPrintLn(__FILE__,
      "Method: GetFileContents(const char* filename) errno:", errno);
    return stringstream{ "" };
  }

  stringstream GetFileContents(std::string filename)
  {
    return GetFileContents(filename.c_str());
  }

  // takes a and a name and returns true if it exists
  bool CheckFileExist(const string path, const string fname, const string ext)
  {
    return filesystem::exists(path + fname + "." + ext);
  }

  // LoadLevel parses a level file, loads models, and creates a level struct from
  // it. should global startup produce a list of files in the level dir?

} // namespace slurp
