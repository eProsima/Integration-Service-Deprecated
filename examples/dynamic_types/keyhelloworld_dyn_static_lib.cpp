#include <iostream>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicDataPtr.h>
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

static DynamicPubSubType* GetHelloWorldType()
{
    // Create basic types
    DynamicTypeBuilder_ptr created_type_ulong = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Builder();
    DynamicTypeBuilder_ptr created_type_string = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder();
    DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    // Add members to the struct.
    struct_type_builder->AddMember(0, "index", created_type_ulong.get());
    struct_type_builder->AddMember(1, "message", created_type_string.get());
    struct_type_builder->SetName("HelloWorld");

    DynamicType_ptr dynType = struct_type_builder->Build();
    DynamicPubSubType *psType = new DynamicPubSubType(dynType);
    return psType;
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

    // DynamicTypes
    DynamicPubSubType *hwType = GetHelloWorldType();
    DynamicData_ptr hwData = DynamicDataFactory::GetInstance()->CreateData(hwType->GetDynamicType());

    // Custom transformation
	hwData->SetUint32Value(keyData.index(), 0);

    // Serialize HelloWorld
    outputBuffer->reserve(static_cast<uint32_t>(hwType->getSerializedSizeProvider(&hwData)()));
    hwType->serialize(hwData.get(), outputBuffer);

    delete hwType;
}

extern "C" USER_LIB_EXPORT void HelloWorldToKey(SerializedPayload_t* inputBuffer, SerializedPayload_t* outputBuffer)
{
    // DynamicTypes
    DynamicPubSubType *hwType = GetHelloWorldType();
    DynamicData_ptr hwData = DynamicDataFactory::GetInstance()->CreateData(hwType->GetDynamicType());
    hwType->deserialize(inputBuffer, hwData.get());

    // Output key Data
    samplePubSubType keyPubSub;
    sample keyData;

	// Custom transformation
    uint32_t temp = hwData->GetUint32Value(0);

    // Custom transformation
    //std::cout << "TRANSFORM: " << temp << std::endl;
    keyData.index() = temp % 256;
    keyData.key_value() = temp % 256;

    // Serialize keys
    outputBuffer->reserve(static_cast<uint32_t>(keyPubSub.getSerializedSizeProvider(&keyData)()));
    keyPubSub.serialize(&keyData, outputBuffer);

    delete hwType;
}