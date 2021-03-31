#pragma once
#include <sstream>
#include <string>

namespace slurp {
std::stringstream get_file_contents(const char*);

bool checkFileExist(const std::string path,
                    const std::string fname,
                    const std::string ext);
}  // namespace slurp