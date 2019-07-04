#include <iostream>
#include <fstream>

#include "winDIBbitmap.h"

#define DIB_HEADER_SIZE 40
#define BMP_HEADER_SIZE 14
#define BYTE_ROW_ALIGNMENT_MULTIPLES 4
#define DWORD_BIT_SIZE 32
#define BMP_BITS_PER_BYTE 8

WINDIBBitmap::WINDIBBitmap() {

}

WINDIBBitmap::~WINDIBBitmap() {

}

uint32_t WINDIBBitmap::writeBMPToFile(uint8_t *inputArray, uint32_t inputArraySizeInBytes, uint32_t imageWidthPixels, uint32_t imageHeightPixels, uint8_t bitsPerPixel) {

	std::ofstream outputStream;

	outputStream.open("test.bmp", std::ios::out | std::ios::binary);

	if (outputStream.fail()) {
		std::cout << "Failed to open test.bmp\n";

		return 1;
	}

	int bytesPerPixel = (bitsPerPixel / BMP_BITS_PER_BYTE);

	int bytesPerPaddedRow = ceil(((float)bitsPerPixel * (float)imageWidthPixels) / (float)DWORD_BIT_SIZE) * BYTE_ROW_ALIGNMENT_MULTIPLES;
	int outputPixelArraySizeInBytes = bytesPerPaddedRow * abs((float)imageHeightPixels);

	int bytesPaddedPerRow = bytesPerPaddedRow - (bytesPerPixel * imageWidthPixels);
	int bytesPerUnpaddedRow = bytesPerPaddedRow - bytesPaddedPerRow;

	std::unique_ptr<char[]> outputPixelsArray(new char[outputPixelArraySizeInBytes]());

	//int outIndexOffset = 0;
	//Note, checking inputArrayIndex against input array index size + 1. Testing against the size + 1 allows the
	//loop to tack on the last padding bytes.
	for (uint32_t inputArrayIndex = 0, _outIndexOffset = 0; inputArrayIndex < inputArraySizeInBytes + 1; inputArrayIndex++) {

		//insert last padding
		if (inputArrayIndex == inputArraySizeInBytes) {
			for (int i = 0; i < bytesPaddedPerRow; i++, _outIndexOffset++) {
				outputPixelsArray[inputArrayIndex + _outIndexOffset] = 0x00;
			}
			break;
		}

		if (inputArrayIndex > 0 && inputArrayIndex % (bytesPerPaddedRow - bytesPaddedPerRow) == 0) {

			//insert padding
			for (int i = 0; i < bytesPaddedPerRow; i++, _outIndexOffset++) {
				outputPixelsArray[inputArrayIndex + _outIndexOffset] = 0x00;
			}
		}

		outputPixelsArray[inputArrayIndex + _outIndexOffset] = inputArray[inputArrayIndex];
	}

	//for (int i = 0; i < outputPixelArraySizeInBytes; i++) {
	//	std::cout << "i: " << i << " " << outputPixelsArray[i] << "\n";
	//}

	WinDIBFormat bmpHeader;

	//Using a pointer to address struct elements directly does not work
	//due to how the compiler may pad the struct. So brute force instead of clever...
	//winDIBFormat* pointerToBmpHeader = &bmpHeader;

	bmpHeader.bmpSize = BMP_HEADER_SIZE + DIB_HEADER_SIZE + outputPixelArraySizeInBytes;
	bmpHeader.pixelArrayOffset = BMP_HEADER_SIZE + DIB_HEADER_SIZE;
	bmpHeader.dibHeaderSize = DIB_HEADER_SIZE;
	bmpHeader.bmpWidth = imageWidthPixels;
	bmpHeader.bmpHeight = imageHeightPixels;
	bmpHeader.bitsPerPixel = bitsPerPixel;
	bmpHeader.rawBmpDataSize = outputPixelArraySizeInBytes;

	uint32_t bitmapArraySizeInBytes = sizeof(bmpHeader);
	bitmapArraySizeInBytes += outputPixelArraySizeInBytes;
	bitmapArraySizeInBytes -= sizeof(bmpHeader.pixelArrayPointer);

	std::unique_ptr<char[]> combinedHeader(new char[bitmapArraySizeInBytes]());

	combinedHeader[sizeof(bmpHeader)] = { 0 };

	uint32_t combinedHeaderIndex = 0;

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.idField, sizeof(bmpHeader.idField));

	combinedHeaderIndex += sizeof(bmpHeader.idField);

	std::memcpy(combinedHeader.get() + combinedHeaderIndex, &bmpHeader.bmpSize, sizeof(bmpHeader.bmpSize));

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