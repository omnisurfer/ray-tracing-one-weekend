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
	//https://en.wikipedia.org/wiki/BMP_file_format
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

int writeBMPToFile() {

	std::ofstream outputStream;

	outputStream.open("test.bmp", std::ios::out | std::ios::binary);

	if (outputStream.fail()) {
		std::cout << "Failed to open test.bmp\n";

		return 1;
	}
	
	char pixels[] = { 
		//(0,0)				//(0,1)				//padding for this row					
		0x00, 0x00, 0xde,	0xad, 0xbe, 0xef,	0x00, 0x00,					
		//(1,0)				//(1,1)				//padding for this row
		0xba, 0x00, 0x00,	0x00, 0xbe, 0x00,	0x00, 0x00 
	};

	char testInputPixels[] = {
		//(0,0)				//(0,1)			
		0x00, 0x00, 0xde,	0xad, 0xbe, 0xef,
		//(1,0)				//(1,1)			
		0xba, 0x00, 0x00,	0x00, 0xbe, 0x00
	};

	int testPixelWidth = 2;

	int byteRowAlignMultiples = 4;
	int dwordBitSize = 32;
	int bitsPerPixel = 24;
	int bitsPerByte = 8;
	int bytesPerPixel = (bitsPerPixel / bitsPerByte);

	int rowSize = ceil( ((float)bitsPerPixel * (float)testPixelWidth) / (float)dwordBitSize ) * byteRowAlignMultiples;	
	int pixelArraySize = rowSize * abs((float)testPixelWidth);

	int rowSizeAlignDifference = rowSize - (bytesPerPixel * testPixelWidth);

	winDIBFormat bmpHeader;

	//Using a pointer to address struct elements directly does not work
	//due to how the compiler may pad the struct
	//winDIBFormat* pointerToBmpHeader = &bmpHeader;

	bmpHeader.bmpSize = 70;
	bmpHeader.pixelArrayOffset = 54;
	bmpHeader.dibHeaderSize = 40;
	bmpHeader.bmpWidth = 2;
	bmpHeader.bmpHeight = 2;
	bmpHeader.bitsPerPixel = 24;
	bmpHeader.rawBmpDataSize = 16;

	int chSize = sizeof(bmpHeader);
	chSize += sizeof(pixels);
	chSize -= sizeof(bmpHeader.pixelArrayPointer);

	std::unique_ptr<char[]> combinedHeader(new char[chSize]());
	
	combinedHeader[sizeof(bmpHeader)] = { 0 };

	int combinedHeaderIndex = 0;

	std::memcpy(combinedHeader.get()+combinedHeaderIndex, &bmpHeader.idField, sizeof(bmpHeader.idField));

	combinedHeaderIndex += sizeof(bmpHeader.idField);		

	std::memcpy(combinedHeader.get()+combinedHeaderIndex, &bmpHeader.bmpSize, sizeof(bmpHeader.bmpSize));

	combinedHeaderIndex += sizeof(bmpHeader.bmpSize);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.reserved0, sizeof(bmpHeader.reserved0));

	combinedHeaderIndex += sizeof(bmpHeader.reserved0);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.reserved1, sizeof(bmpHeader.reserved1));

	combinedHeaderIndex += sizeof(bmpHeader.reserved1);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.pixelArrayOffset, sizeof(bmpHeader.pixelArrayOffset));

	combinedHeaderIndex += sizeof(bmpHeader.pixelArrayOffset);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.dibHeaderSize, sizeof(bmpHeader.dibHeaderSize));

	combinedHeaderIndex += sizeof(bmpHeader.dibHeaderSize);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.bmpWidth, sizeof(bmpHeader.bmpWidth));

	combinedHeaderIndex += sizeof(bmpHeader.bmpWidth);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.bmpHeight, sizeof(bmpHeader.bmpHeight));

	combinedHeaderIndex += sizeof(bmpHeader.bmpHeight);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.colorPlanes, sizeof(bmpHeader.colorPlanes));

	combinedHeaderIndex += sizeof(bmpHeader.colorPlanes);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.bitsPerPixel, sizeof(bmpHeader.bitsPerPixel));

	combinedHeaderIndex += sizeof(bmpHeader.bitsPerPixel);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.pixelArrayCompression, sizeof(bmpHeader.pixelArrayCompression));

	combinedHeaderIndex += sizeof(bmpHeader.pixelArrayCompression);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.rawBmpDataSize, sizeof(bmpHeader.rawBmpDataSize));

	combinedHeaderIndex += sizeof(bmpHeader.rawBmpDataSize);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.horizontalRes, sizeof(bmpHeader.horizontalRes));

	combinedHeaderIndex += sizeof(bmpHeader.horizontalRes);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.verticalRes, sizeof(bmpHeader.verticalRes));

	combinedHeaderIndex += sizeof(bmpHeader.verticalRes);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.colorsInPalette, sizeof(bmpHeader.colorsInPalette));

	combinedHeaderIndex += sizeof(bmpHeader.colorsInPalette);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.importantColors, sizeof(bmpHeader.importantColors));

	combinedHeaderIndex += sizeof(bmpHeader.importantColors);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &pixels, sizeof(pixels));

	combinedHeaderIndex += sizeof(pixels);

	//write out the header
	outputStream.write(combinedHeader.get(), combinedHeaderIndex);

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

	writeBMPToFile();

	return 0;
}