#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

#include "vec3.h"
#include "ray.h"

/* https://github.com/petershirley/raytracinginoneweekend
*/
//"screen" resolution
int nx = 1920, ny = 1080;

struct winDIBFormat {
	//BMP Header
	const char idField[2] = { 0x42, 0x4d };
	int bmpSize = 0;
	const char reserved0[2] = { 0x00, 0x00 };
	const char reserved1[2] = { 0x00, 0x00 };
	int pixelArrayOffset = 0;

	//DIB Header
	int dibHeaderSize = 0;
	int bmpWidth = 0;
	int bmpHeight = 0;
	const uint16_t colorPlanes = 1;
	uint16_t bitsPerPixel = 0;
	const int pixelArrayCompression = 0;
	int rawBmpDataSize = 0;
	int horizontalRes = 2835;
	int verticalRes = 2835;
	int colorsInPalette = 0;
	int importantColors = 0;
	int *pixelArrayPointer = 0;
};

bool hit_sphere(const vec3& center, float radius, const ray& r) {
	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = 2.0 * dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a*c;
	return (discriminant > 0.0);
}

vec3 color(const ray& r) {

	if (hit_sphere(vec3(0, 0, -1), 1.0, r))
		return vec3(1, 0, 0);

	vec3 unit_direction = unit_vector(r.direction());
	float t = 0.5*(unit_direction.y() + 1.0);
	return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int writeBMPHeader() {

	std::ofstream outputStream;

	outputStream.open("test.bmp", std::ios::out | std::ios::binary);

	if (outputStream.fail()) {
		std::cout << "Failed to open test.bmp\n";

		return 1;
	}

	char pixels[] = { 0x00, 0x00, 0xde,
					0xad, 0xbe, 0xef,
					0x00, 0x00,
					0xba, 0x00, 0x00,
					0x00, 0xbe, 0x00,
					0x00, 0x00 
	};

	winDIBFormat bmpHeader;

	winDIBFormat* pointerToBmpHeader = &bmpHeader;

	bmpHeader.bmpSize = 70;
	bmpHeader.pixelArrayOffset = 54;
	bmpHeader.dibHeaderSize = 40;
	bmpHeader.bmpWidth = 2;
	bmpHeader.bmpHeight = 2;
	bmpHeader.bitsPerPixel = 24;
	bmpHeader.rawBmpDataSize = 16;

	std::unique_ptr<char[]> combinedHeader(new char[sizeof(bmpHeader)]());

	int headerIndex = 0;

	for (int i = 0; i < sizeof(bmpHeader.idField); i++, headerIndex++) {
		combinedHeader[headerIndex + i] = bmpHeader.idField[i];
	}

	for (int i = 0; i < sizeof(bmpHeader.bmpSize); i++, headerIndex++) {
		combinedHeader[headerIndex + i] = bmpHeader.bmpSize;
		//NOT IMPLEMENTED
	}

	for (int i = 0; i < sizeof(bmpHeader.reserved0); i++) {
		v.push_back(bmpHeader.reserved0[i]);
	}

	for (int i = 0; i < sizeof(bmpHeader.reserved1); i++) {
		v.push_back(bmpHeader.reserved1[i]);
	}

	v.push_back(bmpHeader.pixelArrayOffset);
	v.push_back(bmpHeader.dibHeaderSize);
	v.push_back(bmpHeader.bmpWidth);
	v.push_back(bmpHeader.bmpHeight);
	//	//<< bmp.colorPlanes
	//	//<< bmp.bitsPerPixel
	//	//<< bmp.pixelArrayCompression
	//	//<< bmp.rawBmpDataSize
	//	//<< bmp.horizontalRes
	//	//<< bmp.verticalRes
	//	//<< bmp.colorsInPalette
	//	//<< bmp.importantColors
	//	<< pixel0 << pixel1 << padding0 << pixel2 << pixel3 << padding1;

	outputStream.write(v.data(), v.size());
	
	outputStream.close();

	return 0;
}

int main() {	

#if 0
	std::ofstream outFile;
	
	outFile.open("output.ppm");

	if (outFile.fail()) {
		std::cout << "Failed to open file\n";
		return 1;
	}		

	outFile << "P3\n" << nx << " " << ny << "\n255\n";

	//simple camera setup
	vec3 lower_left_corner(-(nx / 100), -(ny / 100), -1.0);
	vec3 horizontal((2.0*(nx / 100)), 0.0, 0.0);
	vec3 vertical(0.0, (2.0*(ny / 100)), 0.0);
	vec3 origin(0.0, 0.0, 0.0);

	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {			
			float u = float(i) / float(nx);
			float v = float(j) / float(ny);

			ray r(origin, lower_left_corner + u * horizontal + v * vertical);
			vec3 col = color(r);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);
			outFile << ir << " " << ig << " " << ib << "\n";
		}
	}
#endif
	/*std::cout << "Image generation complete, pres any key to continue...\n";
		
	std::cin.get();

	std::cout << "Writing debug bmp file...\n";*/

	writeBMPHeader();

	return 0;
}