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

	ifstream in{inName, ios::binary | ios::in};
	const auto [w,h] = read_ppm_header(in);
	cout << "width: " << w << " height: " << h << endl;
	return img;
}

int main() {
    using namespace std;
    string inName = R"|(..\levels\test2.ppm)|";


	auto img = read_img_from_ppm(inName);
    
	// print out non_0 vals
    for (int i = 0; i < img->w; i++) {
	for (int j = 0; j < img->h; j++) {
	    const auto idx = i + j * img->w;
	    if (img->data[idx].r != 0 | img->data[idx].g != 0 |
		img->data[idx].b != 0) {
		cout << "pixel (" << i << "," << j << ") =";
		cout << img->data[idx].r << " " << (int)img->data[idx].g << " ";
		cout << (int)img->data[idx].b << "\n";
	    }
	}
    }
    return 0;
}
