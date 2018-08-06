#ifndef _IS_PUBLISHER_H_
#define _IS_PUBLISHER_H_

#include "ISBridge.h"
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>

class ISBridge;

/** Base class for publishers. Must know how to write into the destination protocol */
class ISPublisher : public ISBaseClass
{
protected:
    ISBridge *mb_bridge;
public:
    eprosima::fastrtps::TopicDataType *output_type;
    ISPublisher(const std::string &name) : ISBaseClass(name), mb_bridge(nullptr) { };
    virtual ~ISPublisher() = default;

    virtual bool publish(eprosima::fastrtps::rtps::SerializedPayload_t* /*data*/) = 0;
    virtual bool publish(eprosima::fastrtps::types::DynamicData* /*data*/) = 0;

    virtual ISBridge* setBridge(ISBridge *);

    // Forbid copy
    ISPublisher(const ISPublisher&) = delete;
    ISPublisher& operator=(const ISPublisher&) = delete;
};

typedef ISPublisher* (*createPubf_t)(ISBridge *bridge, const char* name,
                                    const std::vector<std::pair<std::string, std::string>> *config);

#endif //_IS_PUBLISHER_H_