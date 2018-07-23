#include <iostream>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicData.h>

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
    //psType->SetDynamicType(dynType);
    return psType;
}

static DynamicPubSubType* GetKeyType()
{
    // Create basic types
    DynamicTypeBuilder_ptr created_type_octet = DynamicTypeBuilderFactory::GetInstance()->CreateByteBuilder();
    DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    // Add members to the struct.
    struct_type_builder->AddMember(0, "index", created_type_octet.get());
    created_type_octet->ApplyAnnotation("@Key", "true");
    struct_type_builder->AddMember(1, "key_value", created_type_octet.get());
    struct_type_builder->SetName("sample");
    struct_type_builder->ApplyAnnotation("@Key", "true");

    DynamicType_ptr dynType = struct_type_builder->Build();
    DynamicPubSubType *psType = new DynamicPubSubType(dynType);
    //psType->SetDynamicType(dynType);
    return psType;
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

extern "C" USER_LIB_EXPORT void KeyToHelloWorld(
        SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output)
{
    // DynamicTypes
    DynamicPubSubType *keyType = GetKeyType();
    DynamicPubSubType *hwType = GetHelloWorldType();

	// User types
	DynamicData *key_data = DynamicDataFactory::GetInstance()->CreateData(keyType->GetDynamicType());
	DynamicPubSubType key_pst;
	DynamicData *helloworld_data = DynamicDataFactory::GetInstance()->CreateData(hwType->GetDynamicType());
	DynamicPubSubType helloworld_pst;

	// Deserialization
	key_pst.deserialize(serialized_input, key_data);

	// Custom transformation
	helloworld_data->SetByteValue(key_data->GetByteValue(0), 0);
    // Ignore key

	// Serialization
	serialized_output->reserve(helloworld_pst.m_typeSize);
	helloworld_pst.serialize(helloworld_data, serialized_output);

    DynamicDataFactory::GetInstance()->DeleteData(key_data);
    DynamicDataFactory::GetInstance()->DeleteData(helloworld_data);
    delete keyType;
    delete hwType;
}

extern "C" USER_LIB_EXPORT void HelloWorldToKey(
        SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output)
{
    // DynamicTypes
    DynamicPubSubType *keyType = GetKeyType();
    DynamicPubSubType *hwType = GetHelloWorldType();

	// User types
	DynamicData *key_data = DynamicDataFactory::GetInstance()->CreateData(keyType->GetDynamicType());
	DynamicPubSubType key_pst;
	DynamicData *helloworld_data = DynamicDataFactory::GetInstance()->CreateData(hwType->GetDynamicType());
	DynamicPubSubType helloworld_pst;

	// Deserialization
	helloworld_pst.deserialize(serialized_input, helloworld_data);

	// Custom transformation
	key_data->SetByteValue(helloworld_data->GetUint32Value(0) % 256, 0);
	key_data->SetByteValue(helloworld_data->GetUint32Value(0) % 256, 1);

	// Serialization
	serialized_output->reserve(key_pst.m_typeSize);
	key_pst.serialize(key_data, serialized_output);

    DynamicDataFactory::GetInstance()->DeleteData(key_data);
    DynamicDataFactory::GetInstance()->DeleteData(helloworld_data);
    delete keyType;
    delete hwType;
}