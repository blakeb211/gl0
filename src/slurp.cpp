#include "headers.h"
#include "log.h"
#include <streambuf>
using namespace std;

namespace slurp
{

stringstream GetFileContents(const std::string filename)
{
	try
	{
		std::ifstream t(filename);
		std::string str;

		t.seekg(0, std::ios::end);
		str.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		// @NOTE: tbh I'm not sure if the std::move here does anything
		return stringstream{std::move(str)};
	}
	catch (...)
	{
		LogPrintLn("Error: GetFileContents() failed with argument:", filename);
	}
	return stringstream{""};
}

// takes a and a name and returns true if it exists
bool CheckFileExist(const string path, const string fname, const string ext)
{
	return filesystem::exists(path + fname + "." + ext);
}

} // namespace slurp
