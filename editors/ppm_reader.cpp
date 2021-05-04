#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include "..\thirdparty\include\glm\glm.hpp"
#include <memory>
#include <array>

struct RGB
{
	unsigned char r,g,b;
};

struct ImageRGB {
	int w,h;
	std::vector<RGB> data;
};



auto read_ppm_header(std::ifstream & in) {
	using namespace std;
	array<string, 4> header;
	string str_;
	int hdr_item_count{};
	while(true) {
		in >> str_;	
		// skip comments
		if (str_.find("#") == 0) {
			getline(in,str_);
			str_.clear();
			continue;
		}
		header[hdr_item_count] = str_;
		if (hdr_item_count == 3) { break;}
		hdr_item_count++;
		str_.clear();
	}
	in.seekg(0, ios_base::beg);	
	return header;
}

std::unique_ptr<ImageRGB> read_ppm() {
	return std::make_unique<ImageRGB>(ImageRGB{}); 
}

int main() {	
	using namespace std;
	const string inName = R"|(..\levels\test.ppm)|";	
	ifstream inFile(inName, ios::in);


	/**************************************/
	// Read PPM Header
	/**************************************/
	const auto hdrFields = array<string, 4> {"ppmType", "width", "height", "maxColVal"};
	const auto hdr = read_ppm_header(inFile);
	cout << "printing header:" << endl;
	for (int i =0; i < 4; i++) {
		cout << hdrFields[i] << ":" << hdr[i] << "\t";
	}
	cout << endl;
	const int width = stoi(hdr[1]);
	const int height = stoi(hdr[2]);	

	/**************************************/
	// Read PPM length 
	/**************************************/
	inFile.seekg(0, ios_base::beg);	
	inFile.seekg(0, ios_base::end);	
	size_t length = inFile.tellg();
	cout << "file length (bytes):" << length << endl;
	inFile.seekg(0, ios_base::beg);	

	vector<unsigned char> buffer{};
	const auto hdr_length = length - (size_t)(width * height * 3);
	buffer.resize(length - hdr_length);
	
	char c_{}; 
	unsigned char b_{};
	inFile.seekg(0, ios_base::beg);
	inFile.seekg(60, ios_base::beg);
	
	while(inFile.good()) {
		inFile.read(&c_, 1);
		b_ = static_cast<unsigned char>(c_) ;
		if (b_ < 256 && b_ > -1)
		buffer.push_back(c_);
	}

	cout << "buffer.size()" << buffer.size() << endl;

	for (int i = 0; i < buffer.size()-2; i+=3)
	{
	    auto r =static_cast<int>(buffer[i]);
	    auto g =static_cast<int>(buffer[i+1]);
	    auto b =static_cast<int>(buffer[i+2]);
		if (r != 0 || g != 0 || b != 0) 
		{
		cout << r << " " << g << " " << b << " " << endl;
		}
	}

	inFile.close();
	return 0;
}
