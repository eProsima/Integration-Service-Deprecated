#ifndef _EPROSIMA_IS_LOG_H_
#define _EPROSIMA_IS_LOG_H_

#include <iostream>

#ifndef LOG_LEVEL_DEBUG
/* #undef LOG_LEVEL_DEBUG */
#endif

#ifndef LOG_LEVEL_INFO
/* #undef LOG_LEVEL_INFO */
#endif

#ifndef LOG_LEVEL_WARN
/* #undef LOG_LEVEL_WARN */
#endif

#ifndef LOG_LEVEL_ERROR
#define LOG_LEVEL_ERROR
#endif

#if defined _DEBUG || defined DEBUG
    #ifndef LOG_LEVEL_DEBUG
    #define LOG_LEVEL_DEBUG
    #endif
#endif

#ifdef LOG_LEVEL_DEBUG
#define LOG_DEBUG(x) std::cout << "DEBUG: " << x << std::endl
#define LOG_INFO(x) std::cout << "INFO: " << x << std::endl
#define LOG_WARN(x) std::cout << "WARN: " << x << std::endl
#define LOG_ERROR(x) std::cout << "ERROR: " << x << std::endl
#else
#define LOG_DEBUG(x)
#ifdef LOG_LEVEL_INFO
#define LOG_INFO(x) std::cout << "INFO: " << x << std::endl
#define LOG_WARN(x) std::cout << "WARN: " << x << std::endl
#define LOG_ERROR(x) std::cout << "ERROR: " << x << std::endl
#else
#define LOG_INFO(x)
#ifdef LOG_LEVEL_WARN
#define LOG_WARN(x) std::cout << "WARN: " << x << std::endl
#define LOG_ERROR(x) std::cout << "ERROR: " << x << std::endl
#else
#define LOG_WARN(x)
#ifdef LOG_LEVEL_ERROR
#define LOG_ERROR(x) std::cout << "ERROR: " << x << std::endl
#else
#define LOG_ERROR(x)
#endif
#endif
#endif
#endif

#define LOG(x) std::cout << "LOG: " << x << std::endl

#endif
