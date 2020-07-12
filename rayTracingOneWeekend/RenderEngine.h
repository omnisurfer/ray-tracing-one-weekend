#pragma once

#include <iostream>
#include <string>

#include "defines.h"


class MyClass {
public:
	int myNum = 0;
	std::string myString = "Hello World";

	MyClass(void);
	~MyClass(void);

	void SomeFunction(void);
};

class RenderEngine {
private:
	struct RenderFrameProperties {
		uint32_t resWidthInPixels, resHeightInPixels;
		uint8_t bytesPerPixel;
		uint32_t antiAliasingSamplesPerPixel;
		uint32_t imageBufferSizeInBytes;
	};

	RenderFrameProperties *renderFrameProperties;

public:

	RenderEngine(void);
	~RenderEngine(void);

	void ConfigureRenderProperties(void);
	void DisplayRenderProperties(void);
};