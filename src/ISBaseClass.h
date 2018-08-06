#ifndef _IS_BASE_CLASS_H_
#define _IS_BASE_CLASS_H_

#include <fastrtps/types/DynamicData.h>
#include "GenericPubSubTypes.h"
#include <string>

using namespace eprosima::fastrtps::types;
using namespace eprosima::fastrtps::rtps;

typedef void (*userdynf_t)(DynamicData *input, DynamicData *output);
typedef void (*userf_t)(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output);
typedef TopicDataType* (*typef_t)(const char *name);

class ISBaseClass
{
protected:
    std::string name;
    virtual void setName(const std::string &name) { this->name = name; }
public:
    ISBaseClass(const std::string &name) : name(name) {};
    virtual const std::string& getName() const { return name; }
    virtual void onTerminate() {};
    virtual ~ISBaseClass() = default;
};

#endif //_IS_BASE_CLASS_H_