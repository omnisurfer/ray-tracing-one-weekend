#pragma once

#include <iostream>

static uint32_t colorCallCount = 0;

#define DEBUG_MSG_EN 1
#define DEBUG_MSG_LVL_0 1

#if defined DEBUG_MSG_EN && DEBUG_MSG_EN == 1

	#if defined DEBUG_MSG_LVL_0 && DEBUG_MSG_LVL_0 == 1

	#define DEBUG_MSG_L0(functionName, coutLine)			\
		std::cout << functionName << " " << coutLine << "\n";	\

	#endif

#endif