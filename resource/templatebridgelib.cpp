#include <iostream>
#include "ISBridgeDummy.h"

#if defined(_WIN32) && defined (BUILD_SHARED_LIBS)
	#if defined (_MSC_VER)
		#pragma warning(disable: 4251)
	#endif
  #if defined(integration_services_EXPORTS)
  	#define  USER_LIB_EXPORT __declspec(dllexport)
  #else
    #define  USER_LIB_EXPORT __declspec(dllimport)
  #endif
#else
  #define USER_LIB_EXPORT
#endif

ISBridgeDummy* loadDummyBridge(void *bridge_config);

// TODO ISBridge must be now an interface (or abstract class) that all Bridges must implement.
// bridge_config is the string in the xml element <bridge_configuration>
extern "C" ISBridge* USER_LIB_EXPORT createBridge(const char *bridge_config)
{
    // Add Bridge constructor and...
    //return new ISBridgeDummy(bridge_config);

    // OR configure here and return a cleaner Bridge class!
    //return loadDummyBridge(bridge_config);
}

// TODO parse the config and return a configured ISBRidgeDummy
ISBridgeDummy* loadDummyBridge(const char *bridge_config)
{
    try
    {
        ISBRidgeDummyConfig* config = ISBRidgeDummyConfig::Parse(bridge_config); // Or open file, etc.

        // TODO Parse and configure
        // [...]

        ISBridgeDummy *bridge = new ISBridgeDummy(config);

        return bridge;
    }
    catch (int e_code){
        std::cout << "Invalid configuration, skipping bridge " << e_code << std::endl;
    }
}

