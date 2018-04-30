#ifndef _EPROSIMA_IS_LOG_H_
#define _EPROSIMA_IS_LOG_H_

#include <iostream>

#if defined _DEBUG || defined DEBUG
#define LOG_LEVEL_DEBUG
#endif

#ifdef LOG_LEVEL_DEBUG
#define LOG_DEBUG(x) std::cout << "DEBUG: " << x << std::endl
#else
#define LOG_DEBUG(x)
#endif

#ifdef LOG_LEVEL_ERROR
#define LOG_ERROR(x) std::cout << "ERROR: " << x << std::endl
#else
#define LOG_ERROR(x)
#endif

#ifdef LOG_LEVEL_WARN
#define LOG_WARN(x) std::cout << "WARN: " << x << std::endl
#else
#define LOG_WARN(x)
#endif

#ifdef LOG_LEVEL_INFO
#define LOG_INFO(x) std::cout << "INFO: " << x << std::endl
#else
#define LOG_INFO(x)
#endif

#define LOG(x) std::cout << "LOG: " << x << std::endl

#endif
