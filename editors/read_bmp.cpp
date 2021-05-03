#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string> 

struct bitmap {
	unsigned int width, height;
	unsigned char* pixels;
};

struct bitmap* bmp;

void FreeBmp() {
	if (bmp && bmp->pixels) {
		bmp->width = 0;
		bmp->height = 0;
		delete(bmp->pixels);
		delete(bmp);
		bmp = nullptr;
	}
}

bool LoadBmp(const std::string filepath) {
	bmp = nullptr;
	FILE* f = fopen(filepath.c_str(), "rb");

	if (f) {
		bmp = new bitmap();
		bmp->width = 0;
		bmp->height = 0;
		bmp->pixels = nullptr;

		unsigned char info[54] = {0};
		fread(info, sizeof(unsigned char), 54, f);

		bmp->width = *(unsigned int*)&info[18];
		bmp->height = *(unsigned int*)&info[22];

		unsigned int size =
			((((bmp->width * bmp->height) + 31) & ~31) / 8) * bmp->height;
		std::cout << "width of bmp being read:" << bmp->width << std::endl;
		std::cout << "height of bmp being read:" << bmp->height << std::endl;
		std::cout << "size of bmp being read:" << size << std::endl;
		bmp->pixels = new unsigned char[size];
		fread(bmp->pixels, sizeof(unsigned char), size, f);
		fclose(f);

		for (int i = 0; i < size; i += 3) {
			unsigned char tmp = bmp->pixels[i];
			bmp->pixels[i] = bmp->pixels[i + 2];
			bmp->pixels[i + 2] = tmp;
		}
		return true;
	}

	return false;
}

int main() {
	using namespace std;
	string name = R"|(..\levels\test.bmp)|";
	if (LoadBmp(name)) {
		cout << "bitmap " << name << " successfully loaded." << endl;
	}
	unsigned count {0};
	for (int i = 0; i < bmp->width; i++) {
		for (int j = 0; j < bmp->height; j++) {
			count++;
			if (count % 100 == 0) {
				cout << to_string((unsigned int)bmp->pixels[i + j*bmp->width]);
			}
		}
	}
	//....
	FreeBmp();
}
