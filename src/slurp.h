#pragma once
#include "headers.h"
namespace slurp {
  std::stringstream GetFileContents(const char*);

  bool CheckFileExist(const std::string path,
    const std::string fname,
    const std::string ext);
} // namespace slurp
