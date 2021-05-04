#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "..\thirdparty\include\glm\glm.hpp"

struct RGB {
    unsigned char r, g, b;
};

struct ImageRGB {
    int w, h;
    std::vector<RGB> data;
};

std::pair<int,int> read_ppm_header(std::ifstream& in) {
    using namespace std;
    array<string, 4> header;
    string str_;
    int hdr_item_count{};
    while (true) {
	in >> str_;
	// skip comments
	if (str_.find("#") == 0) {
	    getline(in, str_);
	    str_.clear();
	    continue;
	}
	header[hdr_item_count] = str_;
	if (hdr_item_count == 3) {
	    break;
	}
	hdr_item_count++;
	str_.clear();
    }
    in.seekg(0, ios_base::beg);
    return make_pair<int,int>(stoi(header[1]), stoi(header[2]));
}

std::unique_ptr<ImageRGB> read_img_from_ppm(std::string inName) {
    using namespace std;
	auto img = make_unique<ImageRGB>(ImageRGB{});

	assert(!inName.empty());
	ifstream in{inName, ios::binary | ios::in};
	
	const auto [w,h] = read_ppm_header(in);
	img->w = w;
	img->h = h;

	const int dat_len = w * h * 3u;

	namespace fs = std::filesystem;
	const int file_len = fs::file_size(inName); 

	const int hdr_len = file_len - dat_len;
	assert(hdr_len > 0);

	in.seekg(-dat_len, ios_base::end);

	// read data into img->data	
	char c[3]{};
	RGB _rgb{};
	int pix_read{};

	for (int i = 0; i < dat_len - 1; i+=3) {
	in.read(c, 3);
	_rgb.r = static_cast<unsigned char>(c[0]);
	_rgb.g = static_cast<unsigned char>(c[1]);
	_rgb.b = static_cast<unsigned char>(c[2]);
	img->data.push_back(_rgb);
	pix_read++;
	}
	assert(pix_read == img->w * img->h);
	return img;
}

int main() {
    using namespace std;
    string inName = R"|(..\levels\test3.ppm)|";

	auto img = read_img_from_ppm(inName);
	const auto & w = img->w;    
	const auto & h = img->h;    

	cout << " width:" << w << " height: " << h << endl;
	cout << " numPix:" << w * h << " == img.data.size: " << img->data.size() << endl;;
    return 0;
}
