#pragma once
#include <string>

namespace slurp {
std::stringstream get_file_contents(const char*);

bool isFileExists(std::string);
}  // namespace slurp