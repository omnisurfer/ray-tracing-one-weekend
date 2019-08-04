#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

struct RenderProperties {
	uint32_t resWidthInPixels, resHeightInPixels;
	uint8_t bytesPerPixel;
	uint32_t antiAliasingSamplesPerPixel;
	uint32_t finalImageBufferSizeInBytes;
};

struct WorkerImageBuffer {
	uint32_t sizeInBytes;
	uint32_t resWidthInPixels, resHeightInPixels;
	std::shared_ptr<uint8_t> buffer;
};

struct WorkerThread {
	uint32_t id;
	bool workIsDone;
	std::mutex workIsDoneMutex;
	std::condition_variable workIsDoneConditionVar;
	bool exit;
	std::mutex exitMutex;
	std::condition_variable exitConditionVar;
	bool start;
	std::mutex startMutex;
	std::condition_variable startConditionVar;
	std::thread handle;
};
