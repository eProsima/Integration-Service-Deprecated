#include <iostream>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include "idl/HelloWorldPubSubTypes.h"
#include "idl/samplePubSubTypes.h"

#if defined(_WIN32) && defined (BUILD_SHARED_LIBS)
#if defined (_MSC_VER)
#pragma warning(disable: 4251)
#endif
#if defined(EPROSIMA_SHAPES_DLL_EXPORT)
#define  USER_LIB_EXPORT  __declspec(dllexport)
#else
#define  USER_LIB_EXPORT  __declspec(dllimport)
#endif
#else
#define USER_LIB_EXPORT
#endif

using eprosima::fastrtps::TopicDataType;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::types;

static TopicDataType* GetHelloWorldType()
{
    return new HelloWorldPubSubType();
}

static TopicDataType* GetKeyType()
{
    return new samplePubSubType();
}

extern "C" USER_LIB_EXPORT TopicDataType* GetTopicType(const char *name)
{
    if (strncmp(name, "HelloWorld", 10) == 0)
    {
        return GetHelloWorldType();
    }
    else if (strncmp(name, "sample", 6) == 0)
    {
        return GetKeyType();
    }
    return nullptr;
}

extern "C" USER_LIB_EXPORT void KeyToHelloWorld(SerializedPayload_t* inputBuffer, SerializedPayload_t* outputBuffer)
{
    // Input key Data
    samplePubSubType keyPubSub;
    sample keyData;
    keyPubSub.deserialize(inputBuffer, &keyData);

    // Output HelloWorld Data
    HelloWorldPubSubType hwPubSub;
    HelloWorld hwData;

    // Custom transformation
    hwData.index() = keyData.index();
    hwData.message() = "Transform";

    std::cout << "HELLOWORLD INDEX: " << hwData.index() << std::endl;

    // Serialize helloworld
    *outputBuffer = SerializedPayload_t(static_cast<uint32_t>(hwPubSub.getSerializedSizeProvider(&hwData)()));
    hwPubSub.serialize(&hwData, outputBuffer);
}

extern "C" USER_LIB_EXPORT void HelloWorldToKey(SerializedPayload_t* inputBuffer, SerializedPayload_t* outputBuffer)
{
    // Input HelloWorld Data
    HelloWorldPubSubType hwPubSub;
    HelloWorld hwData;
    hwPubSub.deserialize(inputBuffer, &hwData);

    // Input key Data
    samplePubSubType keyPubSub;
    sample keyData;

    // Custom transformation
    keyData.index() = hwData.index() % 256;
    keyData.key_value() = hwData.index() % 256;

    std::cout << "KEY INDEX: " << (int) keyData.index() << std::endl;

    // Serialize helloworld
    *outputBuffer = SerializedPayload_t(static_cast<uint32_t>(keyPubSub.getSerializedSizeProvider(&keyData)()));
    keyPubSub.serialize(&keyData, outputBuffer);
}