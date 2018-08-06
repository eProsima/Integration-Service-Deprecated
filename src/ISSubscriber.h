#ifndef _IS_SUBSCRIBER_H_
#define _IS_SUBSCRIBER_H_

#include "ISBridge.h"
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>

class ISBridge;

/** Base class for subscribers. Must know how to read from the origin protocol */
class ISSubscriber : public ISBaseClass
{
protected:
    std::vector<ISBridge*> mv_bridges;
public:
    eprosima::fastrtps::TopicDataType *input_type;
    ISSubscriber(const std::string &name) : ISBaseClass(name) { };
    virtual ~ISSubscriber() = default;
    virtual void addBridge(ISBridge* bridge);
    virtual void on_received_data(eprosima::fastrtps::rtps::SerializedPayload_t* payload);
    virtual void on_received_data(eprosima::fastrtps::types::DynamicData* data);

    // Forbid copy
    ISSubscriber(const ISSubscriber&) = delete;
    ISSubscriber& operator=(const ISSubscriber&) = delete;
};

typedef ISSubscriber* (*createSubf_t)(ISBridge *bridge, const char* name,
                                      const std::vector<std::pair<std::string, std::string>> *config);

#endif // _IS_SUBSCRIBER_H_