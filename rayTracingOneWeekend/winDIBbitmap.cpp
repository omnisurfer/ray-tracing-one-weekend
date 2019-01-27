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

	std::ofstream _outputStream;

	_outputStream.open("test.bmp", std::ios::out | std::ios::binary);

	if (_outputStream.fail()) {
		std::cout << "Failed to open test.bmp\n";

		return 1;
	}

	int _bytesPerPixel = (bitsPerPixel / BMP_BITS_PER_BYTE);

	int _bytesPerPaddedRow = ceil(((float)bitsPerPixel * (float)imageWidthPixels) / (float)DWORD_BIT_SIZE) * BYTE_ROW_ALIGNMENT_MULTIPLES;
	int _outputPixelArraySizeInBytes = _bytesPerPaddedRow * abs((float)imageHeightPixels);

	int _bytesPaddedPerRow = _bytesPerPaddedRow - (_bytesPerPixel * imageWidthPixels);
	int _bytesPerUnpaddedRow = _bytesPerPaddedRow - _bytesPaddedPerRow;

	std::unique_ptr<char[]> _outputPixelsArray(new char[_outputPixelArraySizeInBytes]());

	//int outIndexOffset = 0;
	//Note, checking inputArrayIndex against input array index size + 1. Testing against the size + 1 allows the
	//loop to tack on the last padding bytes.
	for (uint32_t _inputArrayIndex = 0, _outIndexOffset = 0; _inputArrayIndex < inputArraySizeInBytes + 1; _inputArrayIndex++) {

		//insert last padding
		if (_inputArrayIndex == inputArraySizeInBytes) {
			for (int i = 0; i < _bytesPaddedPerRow; i++, _outIndexOffset++) {
				_outputPixelsArray[_inputArrayIndex + _outIndexOffset] = 0x00;
			}
			break;
		}

		if (_inputArrayIndex > 0 && _inputArrayIndex % (_bytesPerPaddedRow - _bytesPaddedPerRow) == 0) {

			//insert padding
			for (int i = 0; i < _bytesPaddedPerRow; i++, _outIndexOffset++) {
				_outputPixelsArray[_inputArrayIndex + _outIndexOffset] = 0x00;
			}
		}

		_outputPixelsArray[_inputArrayIndex + _outIndexOffset] = inputArray[_inputArrayIndex];
	}

	//for (int i = 0; i < outputPixelArraySizeInBytes; i++) {
	//	std::cout << "i: " << i << " " << outputPixelsArray[i] << "\n";
	//}

	WinDIBFormat _bmpHeader;

	//Using a pointer to address struct elements directly does not work
	//due to how the compiler may pad the struct. So brute force instead of clever...
	//winDIBFormat* pointerToBmpHeader = &bmpHeader;

	_bmpHeader.bmpSize = BMP_HEADER_SIZE + DIB_HEADER_SIZE + _outputPixelArraySizeInBytes;
	_bmpHeader.pixelArrayOffset = BMP_HEADER_SIZE + DIB_HEADER_SIZE;
	_bmpHeader.dibHeaderSize = DIB_HEADER_SIZE;
	_bmpHeader.bmpWidth = imageWidthPixels;
	_bmpHeader.bmpHeight = imageHeightPixels;
	_bmpHeader.bitsPerPixel = bitsPerPixel;
	_bmpHeader.rawBmpDataSize = _outputPixelArraySizeInBytes;

	uint32_t _bitmapArraySizeInBytes = sizeof(_bmpHeader);
	_bitmapArraySizeInBytes += _outputPixelArraySizeInBytes;
	_bitmapArraySizeInBytes -= sizeof(_bmpHeader.pixelArrayPointer);

	std::unique_ptr<char[]> _combinedHeader(new char[_bitmapArraySizeInBytes]());

	_combinedHeader[sizeof(_bmpHeader)] = { 0 };

	uint32_t _combinedHeaderIndex = 0;

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.idField, sizeof(_bmpHeader.idField));

	_combinedHeaderIndex += sizeof(_bmpHeader.idField);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.bmpSize, sizeof(_bmpHeader.bmpSize));

	_combinedHeaderIndex += sizeof(_bmpHeader.bmpSize);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.reserved0, sizeof(_bmpHeader.reserved0));

	_combinedHeaderIndex += sizeof(_bmpHeader.reserved0);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.reserved1, sizeof(_bmpHeader.reserved1));

	_combinedHeaderIndex += sizeof(_bmpHeader.reserved1);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.pixelArrayOffset, sizeof(_bmpHeader.pixelArrayOffset));

	_combinedHeaderIndex += sizeof(_bmpHeader.pixelArrayOffset);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.dibHeaderSize, sizeof(_bmpHeader.dibHeaderSize));

	_combinedHeaderIndex += sizeof(_bmpHeader.dibHeaderSize);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.bmpWidth, sizeof(_bmpHeader.bmpWidth));

	_combinedHeaderIndex += sizeof(_bmpHeader.bmpWidth);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.bmpHeight, sizeof(_bmpHeader.bmpHeight));

	_combinedHeaderIndex += sizeof(_bmpHeader.bmpHeight);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.colorPlanes, sizeof(_bmpHeader.colorPlanes));

	_combinedHeaderIndex += sizeof(_bmpHeader.colorPlanes);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.bitsPerPixel, sizeof(_bmpHeader.bitsPerPixel));

	_combinedHeaderIndex += sizeof(_bmpHeader.bitsPerPixel);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.pixelArrayCompression, sizeof(_bmpHeader.pixelArrayCompression));

	_combinedHeaderIndex += sizeof(_bmpHeader.pixelArrayCompression);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.rawBmpDataSize, sizeof(_bmpHeader.rawBmpDataSize));

	_combinedHeaderIndex += sizeof(_bmpHeader.rawBmpDataSize);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.horizontalRes, sizeof(_bmpHeader.horizontalRes));

	_combinedHeaderIndex += sizeof(_bmpHeader.horizontalRes);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.verticalRes, sizeof(_bmpHeader.verticalRes));

	_combinedHeaderIndex += sizeof(_bmpHeader.verticalRes);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.colorsInPalette, sizeof(_bmpHeader.colorsInPalette));

	_combinedHeaderIndex += sizeof(_bmpHeader.colorsInPalette);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, &_bmpHeader.importantColors, sizeof(_bmpHeader.importantColors));

	_combinedHeaderIndex += sizeof(_bmpHeader.importantColors);

	std::memcpy(_combinedHeader.get() + _combinedHeaderIndex, _outputPixelsArray.get(), _outputPixelArraySizeInBytes);

	_combinedHeaderIndex += _outputPixelArraySizeInBytes;

	//write out the header
	_outputStream.write(_combinedHeader.get(), _combinedHeaderIndex);

	_outputStream.close();

	return 0;
}