#pragma once

#include <iostream>

#define DEBUG_MSG_EN 0
#define DEBUG_MSG_LVL_0_EN 1
#define DEBUG_MSG_FUNC_NAME_EN 1

#if defined DEBUG_MSG_EN && DEBUG_MSG_EN == 1

	#if defined DEBUG_MSG_LVL_0_EN && DEBUG_MSG_LVL_0_EN == 1

	#define DEBUG_MSG_L0(functionName, coutLine)	\
		coutLock.lock();	\
		std::cout << functionName << " " << coutLine << "\n";	\
		coutLock.unlock();
	#else
	#define DEBUG_MSG_L0(functionName, coutLine)
	#endif

	#if defined DEBUG_MSG_FUNC_NAME_EN && DEBUG_MSG_FUNC_NAME_EN 1

	#define DEBUG_MSG_FUNC_NAME(functionName, coutLine)	\
		coutLock.lock();	\
		std::cout << functionName << " " << coutLine << "\n";	\
		coutLock.unlock();

	#else
	#define DEBUG_MSG_FUNC_NAME(functionName, coutLine)
	#endif

#else
#define DEBUG_MSG_L0(functionName, coutLine)
#define DEBUG_MSG_FUNC_NAME(functionName, coutLine)

#endif