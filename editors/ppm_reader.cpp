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

auto read_ppm_header(std::ifstream& in) {
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
    return header;
}

std::unique_ptr<ImageRGB> read_img_from_ppm(std::string inName,
					    bool printHeader = false) {
    using namespace std;

    ifstream inFile(inName, ios::in | ios::binary);

    auto img = make_unique<ImageRGB>(ImageRGB{});

    /**************************************/
    // Read PPM Header
    /**************************************/
    const auto hdrFields =
	array<string, 4>{"ppmType", "width", "height", "maxColVal"};
    const auto hdr = read_ppm_header(inFile);

    if (printHeader) {
	cout << "printing header:" << endl;
	for (int i = 0; i < 4; i++) {
	    cout << hdrFields[i] << ":" << hdr[i] << "\t";
	}
	cout << endl;
    }

    img->w = stoi(hdr[1]);
    img->h = stoi(hdr[2]);

    /**************************************/
    // Read PPM fLen
    /**************************************/
    namespace fs = std::filesystem;
    const auto fLen = fs::file_size(inName);
    const auto hdr_len = fLen - (size_t)(img->w * img->h * 3);

    img->data.reserve(fLen - hdr_len);

    char c_[3]{};
    RGB col_{};
    inFile.seekg(0, ios_base::beg);
    inFile.seekg(hdr_len + 1, ios_base::beg);

    while (true) {
	inFile.read(c_, 3);
	if (!inFile.good()) {
	    break;
	}

	col_.r = static_cast<unsigned char>(c_[0]);
	col_.b = static_cast<unsigned char>(c_[1]);
	col_.g = static_cast<unsigned char>(c_[2]);

	img->data.push_back(col_);
    }

    inFile.close();
    return img;
}

int main() {
    using namespace std;
    string inName = R"|(..\levels\test.ppm)|";
    auto img = read_img_from_ppm(inName);
    cout << "pixels saved to image data: " << img->data.size() << endl;

    // print out non_0 vals
    for (int i = 0; i < img->w; i++) {
	for (int j = 0; j < img->h; j++) {
	    const auto idx = i + j * img->w;
	    if (img->data[idx].r != 0 | img->data[idx].g != 0 |
		img->data[idx].b != 0) {
		cout << "pixel (" << i + 1 << "," << j + 1 << ") =";
		cout << (int)img->data[idx].r << " " << (int)img->data[idx].g << " ";
		cout << (int)img->data[idx].b << "\n";
	    }
	}
    }
    return 0;
}
