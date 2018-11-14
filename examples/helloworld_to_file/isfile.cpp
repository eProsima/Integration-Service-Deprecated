#include <iostream>
#include "FilePublisher.h"
#include "../../src/log/ISLog.h"

#if defined(_WIN32) && defined (BUILD_SHARED_LIBS)
	#if defined (_MSC_VER)
		#pragma warning(disable: 4251)
	#endif
  #if defined(EPROSIMA_USER_DLL_EXPORT)
  	#define  USER_LIB_EXPORT __declspec(dllexport)
  #else
    #define  USER_LIB_EXPORT __declspec(dllimport)
  #endif
#else
  #define USER_LIB_EXPORT
#endif

FilePublisher* loadFilePublisher(const char* name, const std::vector<std::pair<std::string, std::string>> *config);

extern "C" USER_LIB_EXPORT ISBridge* create_bridge(const char*,
    const std::vector<std::pair<std::string, std::string>>*)
{
    return nullptr; // Not implemented
}

extern "C" USER_LIB_EXPORT ISSubscriber* create_subscriber(ISBridge*, const char*,
    const std::vector<std::pair<std::string, std::string>>*)
{
    return nullptr; // Not implemented
}

extern "C" USER_LIB_EXPORT ISPublisher* create_publisher(ISBridge*, const char* name,
    const std::vector<std::pair<std::string, std::string>> *config)
{
    return loadFilePublisher(name, config);
}

FilePublisher* loadFilePublisher(const char* name, const std::vector<std::pair<std::string, std::string>> *config)
{
    if (!config)
    {
        return nullptr;
    }

    FilePublisher* publisher = new FilePublisher(name, config);
    return publisher;
}
