#pragma once
#include <string>
#include <utility>

void load_level(std::string);

std::pair<int, int> extract_pair_of_ints(std::string& token,
                                         std::string& delimiter,
                                         const char*& name);
