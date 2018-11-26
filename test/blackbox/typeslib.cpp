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

static DynamicPubSubType* GetStringType()
{
    // Create basic types
    DynamicTypeBuilder_ptr created_type_string = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder(10000);
    DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    // Add members to the struct.
    struct_type_builder->AddMember(0, "message", created_type_string.get());
    struct_type_builder->SetName("StringType");

    DynamicType_ptr dynType = struct_type_builder->Build();
    DynamicPubSubType *psType = new DynamicPubSubType(dynType);
    return psType;
}

static DynamicPubSubType* GetKeyedType()
{
    // Create basic types
    DynamicTypeBuilder_ptr created_type_ulong = DynamicTypeBuilderFactory::GetInstance()->CreateUint32Builder();
    DynamicTypeBuilder_ptr created_type_string = DynamicTypeBuilderFactory::GetInstance()->CreateStringBuilder();
    DynamicTypeBuilder_ptr struct_type_builder = DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();

    // Add members to the struct.
    created_type_ulong->ApplyAnnotation("@Key", "true");
    struct_type_builder->AddMember(0, "index", created_type_ulong.get());
    struct_type_builder->AddMember(1, "message", created_type_string.get());
    struct_type_builder->SetName("Keyed");
    struct_type_builder->ApplyAnnotation("@Key", "true");

    DynamicType_ptr dynType = struct_type_builder->Build();
    DynamicPubSubType *psType = new DynamicPubSubType(dynType);
    return psType;
}

extern "C" USER_LIB_EXPORT TopicDataType* GetTopicType(const char *name)
{
    if (strncmp(name, "StringType", 10) == 0)
    {
        return GetStringType();
    }
    else if (strncmp(name, "Keyed", 5) == 0)
    {
        return GetKeyedType();
    }
    return nullptr;
}
