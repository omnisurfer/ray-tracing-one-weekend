#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

#include "vec3.h"
#include "ray.h"

#define DIB_HEADER_SIZE 40
#define BMP_HEADER_SIZE 14
#define BYTE_ROW_ALIGNMENT_MULTIPLES 4
#define DWORD_BIT_SIZE 32
#define BITS_PER_BYTE 8

/* https://github.com/petershirley/raytracinginoneweekend
*/
//"screen" resolution
int nx = 3840, ny = 2160;

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

	if (hit_sphere(vec3(0, 0, -1), 0.9, r))
		return vec3(1, 0, 0);

	vec3 unit_direction = unit_vector(r.direction());
	float t = 0.5*(unit_direction.y() + 1.0);
	return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

int writeBMPToFile(uint8_t *inputArray, uint32_t inputArraySizeInBytes, uint32_t imageWidthPixels, uint32_t imageHeightPixels, uint8_t bitsPerPixel) {

	std::ofstream outputStream;

	outputStream.open("test.bmp", std::ios::out | std::ios::binary);

	if (outputStream.fail()) {
		std::cout << "Failed to open test.bmp\n";

		return 1;
	}

	int bytesPerPixel = (bitsPerPixel / BITS_PER_BYTE);

	int bytesPerPaddedRow = ceil( ((float)bitsPerPixel * (float)imageWidthPixels) / (float)DWORD_BIT_SIZE ) * BYTE_ROW_ALIGNMENT_MULTIPLES;	
	int outputPixelArraySizeInBytes = bytesPerPaddedRow * abs((float)imageHeightPixels);

	int bytesPaddedPerRow = bytesPerPaddedRow - (bytesPerPixel * imageWidthPixels);
	int bytesPerUnpaddedRow = bytesPerPaddedRow - bytesPaddedPerRow;

	std::unique_ptr<char[]> outputPixelsArray(new char[outputPixelArraySizeInBytes]());

	//int outIndexOffset = 0;
	//Note, checking inputArrayIndex against input array index size + 1. Testing against the size + 1 allows the
	//loop to tack on the last padding bytes.
	for (int inputArrayIndex = 0, outIndexOffset = 0; inputArrayIndex < inputArraySizeInBytes + 1; inputArrayIndex++) {

		//insert last padding
		if (inputArrayIndex == inputArraySizeInBytes) {
			for (int i = 0; i < bytesPaddedPerRow; i++, outIndexOffset++) {
				outputPixelsArray[inputArrayIndex + outIndexOffset] = 0x00;
			}
			break;
		}

		if (inputArrayIndex > 0 && inputArrayIndex%(bytesPerPaddedRow - bytesPaddedPerRow) == 0) {

			//insert padding
			for (int i = 0; i < bytesPaddedPerRow; i++, outIndexOffset++) {
				outputPixelsArray[inputArrayIndex + outIndexOffset] = 0x00;
			}
		}

		outputPixelsArray[inputArrayIndex + outIndexOffset] = inputArray[inputArrayIndex];
	}

	//for (int i = 0; i < outputPixelArraySizeInBytes; i++) {
	//	std::cout << "i: " << i << " " << outputPixelsArray[i] << "\n";
	//}

	winDIBFormat bmpHeader;

	//Using a pointer to address struct elements directly does not work
	//due to how the compiler may pad the struct
	//winDIBFormat* pointerToBmpHeader = &bmpHeader;

	bmpHeader.bmpSize = BMP_HEADER_SIZE + DIB_HEADER_SIZE + outputPixelArraySizeInBytes;
	bmpHeader.pixelArrayOffset = BMP_HEADER_SIZE + DIB_HEADER_SIZE;
	bmpHeader.dibHeaderSize = DIB_HEADER_SIZE;
	bmpHeader.bmpWidth = imageWidthPixels;
	bmpHeader.bmpHeight = imageHeightPixels;
	bmpHeader.bitsPerPixel = 24;
	bmpHeader.rawBmpDataSize = outputPixelArraySizeInBytes;

	int bitmapArraySizeInBytes = sizeof(bmpHeader);
	bitmapArraySizeInBytes += outputPixelArraySizeInBytes;
	bitmapArraySizeInBytes -= sizeof(bmpHeader.pixelArrayPointer);

	std::unique_ptr<char[]> combinedHeader(new char[bitmapArraySizeInBytes]());
	
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

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, outputPixelsArray.get(), outputPixelArraySizeInBytes);

	combinedHeaderIndex += outputPixelArraySizeInBytes;

	//write out the header
	outputStream.write(combinedHeader.get(), combinedHeaderIndex);

	outputStream.close();

	return 0;
}

int main() {	

	uint32_t tempBufferSizeInBytes = nx * ny * 3;

	uint8_t *tempBuffer = new uint8_t[tempBufferSizeInBytes]();

	for (int i = 0; i < tempBufferSizeInBytes - 1; i++) {
		tempBuffer[i] = 0x24;
		tempBuffer[tempBufferSizeInBytes - 1] = 0x24;
	}

#if 1
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

	for (int row = ny - 1; row >= 0; row--) {
		if (row == 1)
			std::cout << "row is 1\n";
		if (row == 0)
			std::cout << "row is zero\n";

		for (int column = 0; column < nx; column++) {			
			float u = float(column) / float(nx);
			float v = float(row) / float(ny);

			ray r(origin, lower_left_corner + u * horizontal + v * vertical);
			vec3 col = color(r);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);
			outFile << ir << " " << ig << " " << ib << "\n";

			//also store values into tempBuffer
			tempBuffer[row*nx * 3 + (column * 3)] = ib;
			tempBuffer[row*nx * 3 + (column * 3) + 1] = ig;
			tempBuffer[row*nx * 3 + (column * 3) + 2] = ir;
		}
	}
#endif
	std::cout << "Image generation complete, pres any key to continue...\n";
		
	//std::cin.get();

	std::cout << "Writing debug bmp file...\n";

	writeBMPToFile(tempBuffer, tempBufferSizeInBytes, nx, ny, 24);

	delete[] tempBuffer;

	return 0;
}