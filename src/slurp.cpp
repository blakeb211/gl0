#define _CRT_SECURE_NO_WARNINGS
#include <errno.h>  
#include <cstdio>
#include <cstdlib>
#include <string>


std::string get_file_contents(const char *filename) {
  std::FILE *fp = std::fopen(filename, "rb");
  if (fp) {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return (contents);
  }
  // error string if fopen failed
  printf("errno: %d", errno);
  return "";
}

