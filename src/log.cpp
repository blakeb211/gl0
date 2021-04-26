#include "log.h"
//
#include <fmt\core.h>
#include <fmt\format.h>
#include <fmt\os.h>
#include <array>
#include <cstdio>
#include <string>

using namespace std;

constexpr auto BUFFER_LEN = 512;

static FILE* fptr;
array<char, BUFFER_LEN> buf;

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

bool setLogFile(std::string fname) {
    if (fname.empty()) {
	return false;
    }

    fptr = fopen(fname.c_str(), "w+");
    if (fptr != NULL) {
	return true;
    } else {
	return false;
    }
}

template <class T>
void write_to_screen_and_disk(const string fmt, any val) {
    const static string fmt_ = fmt;
    const auto arg_ = any_cast<T>(val);
    auto cnt_ = fmt::formatted_size(fmt_, arg_);
    auto _ = fmt::format_to_n(buf.begin(), cnt_, fmt_, arg_);
    fmt::print(fmt_, arg_);
    fwrite(buf.data(), sizeof(char), cnt_, fptr);
}

void logPrintLn(const initializer_list<any>& il) {
    for (auto& i : il) {
	const string typeName{i.type().name()};

	if (typeid(0.1f) == i.type()) {
	    write_to_screen_and_disk<float>("{:<8.3f} ", i);
	    continue;
	}
	if (typeid(const char*) == i.type()) {
	    write_to_screen_and_disk<const char*>("{} ", i);
	    continue;
	}
	if (typeid(char*) == i.type()) {
	    write_to_screen_and_disk<char*>("{} ", i);
	    continue;
	}
	if (typeid(1) == i.type()) {
	    write_to_screen_and_disk<int>("{:<8d} ", i);
	    continue;
	}
	if (typeid(1.0) == i.type()) {
	    write_to_screen_and_disk<double>("{:<8.3f} ", i);
	    continue;
	}
	if (typeid(1u) == i.type()) {
	    write_to_screen_and_disk<unsigned int>("{:<8d} ", i);
	    continue;
	}
	if (typeid(string("a")) == i.type()) {
	    write_to_screen_and_disk<string>("{} ", i);
	    continue;
	}
	if (typeid(size_t) == i.type()) {
	    write_to_screen_and_disk<size_t>("{:<8d} ", i);
	    continue;
	}
	//****************************************************************
	// add handling for next type here
	//****************************************************************

	write_to_screen_and_disk<const string>(
	    "ERROR: add type <{}> to logPrintLn", typeName);
    }
    // print a new line at the end regardless of what got printed
    write_to_screen_and_disk<const char*>("{}", "\n");
}

void logErr(const string fname, const int lineNum, const string msg) {
    logPrintLn({"ERROR: file <", fname, "> line <", lineNum, "> ==", msg});
}
