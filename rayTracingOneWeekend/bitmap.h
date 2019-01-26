#pragma once
#include <stdlib.h>
#include <stdint.h>

#define DIB_HEADER_SIZE 40
#define BMP_HEADER_SIZE 14
#define BYTE_ROW_ALIGNMENT_MULTIPLES 4
#define DWORD_BIT_SIZE 32
#define BMP_BITS_PER_BYTE 8
#define BMP_BITS_PER_PIXEL 24

/*
	My quickly thrown together WinDIB BMP writer. Just enough to work but not really tested beyond that.
*/
struct WinDIBFormat {
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
	int horizontalRes = 0;
	int verticalRes = 0;
	int colorsInPalette = 0;
	int importantColors = 0;
	int *pixelArrayPointer = 0;
};

class WINDIBBitmap {

public:
	WINDIBBitmap();
	~WINDIBBitmap();

	uint32_t writeBMPToFile(uint8_t *inputArray, uint32_t inputArraySizeInBytes, uint32_t imageWidthPixels, uint32_t imageHeightPixels, uint8_t bitsPerPixel);

protected:

private:

};