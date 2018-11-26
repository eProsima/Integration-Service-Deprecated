#include <iostream>
#include "ShapePubSubTypes.h"

#if defined(_WIN32) && defined (BUILD_SHARED_LIBS)
#if defined (_MSC_VER)
#pragma warning(disable: 4251)
#endif
#if defined(EPROSIMA_SHAPES_DLL_EXPORT)
#define  SHAPES_LIB_EXPORT __declspec(dllexport)
#else
#define  SHAPES_LIB_EXPORT __declspec(dllimport)
#endif
#else
#define SHAPES_LIB_EXPORT
#endif

using eprosima::fastrtps::TopicDataType;

TopicDataType* GetShapeType()
{
    return new ShapeTypePubSubType();
}

extern "C" SHAPES_LIB_EXPORT TopicDataType* GetTopicType(const char *name)
{
    if (strncmp(name, "ShapeType", 9) == 0)
    {
        return GetShapeType();
    }
    return nullptr;
}