#include "RenderEngine.h"

MyClass::MyClass(void) {
	std::cout << "Creating MyClass " << myString << "\n";
}

MyClass::~MyClass(void)
{
	std::cout << "Destroying MyClass\n";
}

void MyClass::SomeFunction(void)
{
	std::cout << "Some Function " << myNum << "\n";
}


//Render Engine

RenderEngine::RenderEngine(void)
{
	renderFrameProperties = new RenderFrameProperties();

	renderFrameProperties->resHeightInPixels = DEFAULT_RENDER_HEIGHT;
	renderFrameProperties->resWidthInPixels = DEFAULT_RENDER_WIDTH;
	renderFrameProperties->antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;
	renderFrameProperties->bytesPerPixel = 32/8; //32-bits per pixel, 8 bits/byte
	renderFrameProperties->imageBufferSizeInBytes = renderFrameProperties->resWidthInPixels * renderFrameProperties->resHeightInPixels * renderFrameProperties->bytesPerPixel;
}

RenderEngine::~RenderEngine(void)
{
}

void RenderEngine::ConfigureRenderProperties(void)
{	

	//ask for image dimensions
	std::cout << "Enter render width: ";
	std::cout.flush();
	std::cin.clear();
	//std::cin.ignore(INT_MAX, '\n');

	if (std::cin.peek() == '\n') {
		std::cout << "Invalid input, using default: " << renderFrameProperties->resWidthInPixels << '\n';
	}
	else {
		std::cin >> renderFrameProperties->resWidthInPixels;

		//drowan(20190607) BUG: If the width and height is set to 1x1, I get heap corruption in the BMP writer. 
		//For now, going to use a minimum value to step around this issue until I can fix it.
		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			renderFrameProperties->resWidthInPixels = DEFAULT_RENDER_WIDTH;
			std::cout << "Invalid input, using default: " << renderFrameProperties->resWidthInPixels << '\n';
		}
		else {
			if (renderFrameProperties->resWidthInPixels < DEFAULT_RENDER_WIDTH) {
				renderFrameProperties->resWidthInPixels = DEFAULT_RENDER_WIDTH;
				std::cout << "Minimum width set: " << DEFAULT_RENDER_WIDTH << "\n";
			}
		}
	}

	std::cout << "Enter render height: ";
	std::cout.flush();
	std::cin.clear();
	std::cin.ignore(INT_MAX, '\n');

	if (std::cin.peek() == '\n') {
		std::cout << "Invalid input, using default: " << renderFrameProperties->resHeightInPixels << '\n';
	}
	else {
		std::cin >> renderFrameProperties->resHeightInPixels;

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			renderFrameProperties->resHeightInPixels = DEFAULT_RENDER_HEIGHT;
			std::cout << "Invalid input, using default: " << renderFrameProperties->resHeightInPixels << '\n';
		}
		else {
			if (renderFrameProperties->resHeightInPixels < DEFAULT_RENDER_HEIGHT) {
				renderFrameProperties->resHeightInPixels = DEFAULT_RENDER_HEIGHT;
				std::cout << "Minimum height set: " << DEFAULT_RENDER_HEIGHT << "\n";
			}
		}
	}

	std::cout << "Enter number of anti-aliasing samples (also helps increase photon count): ";
	std::cout.flush();
	std::cin.clear();
	std::cin.ignore(INT_MAX, '\n');

	if (std::cin.peek() == '\n') {
		std::cout << "Invalid input, using default: " << renderFrameProperties->antiAliasingSamplesPerPixel << '\n';
	}
	else {
		std::cin >> renderFrameProperties->antiAliasingSamplesPerPixel;

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			renderFrameProperties->antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;
			std::cout << "Invalid input, using default: " << renderFrameProperties->antiAliasingSamplesPerPixel << '\n';
		}
		else {
			if (renderFrameProperties->antiAliasingSamplesPerPixel < DEFAULT_RENDER_AA) {
				renderFrameProperties->antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;
				std::cout << "Minimum AA set: " << DEFAULT_RENDER_AA << "\n";
			}
		}
	}

	renderFrameProperties->imageBufferSizeInBytes = renderFrameProperties->resWidthInPixels * renderFrameProperties->resHeightInPixels * renderFrameProperties->bytesPerPixel;
}

void RenderEngine::DisplayRenderProperties(void)
{
	std::cout << "Current render settings: \n" 
		<< "Height: " << renderFrameProperties->resHeightInPixels 
		<< " Width: " << renderFrameProperties->resWidthInPixels 
		<< " AA: " << renderFrameProperties->antiAliasingSamplesPerPixel 
		<< " Bytes/Pixel: " << (int)renderFrameProperties->bytesPerPixel
		<< " Image Buffer Size (bytes): " << renderFrameProperties->imageBufferSizeInBytes
		<< "\n";
}
