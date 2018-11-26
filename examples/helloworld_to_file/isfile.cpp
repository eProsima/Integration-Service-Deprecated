#include <iostream>
#include "FileWriter.h"
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

FileWriter* loadFilePublisher(const char* name, const std::vector<std::pair<std::string, std::string>> *config);

extern "C" USER_LIB_EXPORT ISBridge* create_bridge(const char*,
    const std::vector<std::pair<std::string, std::string>>*)
{
    return nullptr; // Not implemented
}

extern "C" USER_LIB_EXPORT ISReader* create_reader(ISBridge*, const char*,
    const std::vector<std::pair<std::string, std::string>>*)
{
    return nullptr; // Not implemented
}

extern "C" USER_LIB_EXPORT ISWriter* create_writer(ISBridge*, const char* name,
    const std::vector<std::pair<std::string, std::string>> *config)
{
    return loadFilePublisher(name, config);
}

FileWriter* loadFilePublisher(const char* name, const std::vector<std::pair<std::string, std::string>> *config)
{
    if (!config)
    {
        return nullptr;
    }

    FileWriter* publisher = new FileWriter(name, config);
    return publisher;
}
