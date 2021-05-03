#include <iostream>
#include <vector>
#include <string>
#include "..\thirdparty\include\glm\glm.hpp"
#include <memory>

struct RGB
{
	unsigned char r,g,b;
};

struct ImageRGB {
	int w,h;
	std::vector<RGB> data;
};


std::unique_ptr<ImageRGB> read_ppm() {

	return std::make_unique<ImageRGB>(ImageRGB{}); 
}

int main() {	

	return 0;
}
