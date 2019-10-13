#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

//make this global for now
//drowan(20190607): maybe look into this: https://stackoverflow.com/questions/9332263/synchronizing-std-cout-output-multi-thread	
std::mutex globalCoutGuard;

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

struct GUIControlInputs {
	bool forwardAsserted = false;
	bool reverseAsserted = false;
	bool leftAsserted = false;
	bool rightAsserted = false;
	bool escAsserted = false;
};

struct WorkerThread {
	uint32_t id;

	bool start;
	std::mutex startMutex;
	std::condition_variable startConditionVar;
	bool continueWork;
	std::mutex continueWorkMutex;
	std::condition_variable continueWorkConditionVar;
	bool workIsDone;
	std::mutex workIsDoneMutex;
	std::condition_variable workIsDoneConditionVar;
	bool exit;
	std::mutex exitMutex;
	std::condition_variable exitConditionVar;		
	std::thread handle;
	int configuredMaxThreads = std::thread::hardware_concurrency();;
};
