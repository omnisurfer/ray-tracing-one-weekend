#pragma once
#include <stdlib.h>
#include <stdint.h>

#define BMP_BITS_PER_PIXEL 32

/*
	My quickly thrown together WinDIB BMP writer. Just enough to work but not really tested beyond that.
*/
class WINDIBBitmap {

public:
	WINDIBBitmap();
	~WINDIBBitmap();

	static uint32_t writeBMPToFile(uint8_t *inputArray, uint32_t inputArraySizeInBytes, uint32_t imageWidthPixels, uint32_t imageHeightPixels, uint8_t bitsPerPixel);

	static uint32_t getBitsPerPixel() {
		return BMP_BITS_PER_PIXEL;
	}

protected:
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

private:

};