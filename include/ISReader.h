#ifndef _IS_READER_H_
#define _IS_READER_H_

#include "ISBridge.h"
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>

class ISBridge;

/** Base class for readers. They must know how to read from the origin protocol */
class ISReader : public ISBaseClass
{
protected:
    std::vector<ISBridge*> mv_bridges;
public:
    eprosima::fastrtps::TopicDataType *input_type;
    ISReader(const std::string &name) : ISBaseClass(name), input_type(nullptr) { };
    virtual ~ISReader() = default;
    virtual void addBridge(ISBridge* bridge);
    virtual void on_received_data(eprosima::fastrtps::rtps::SerializedPayload_t* payload);
    virtual void on_received_data(eprosima::fastrtps::types::DynamicData* data);

    // Forbid copy
    ISReader(const ISReader&) = delete;
    ISReader& operator=(const ISReader&) = delete;
};

typedef ISReader* (*createSubf_t)(ISBridge *bridge, const char* name,
    const std::vector<std::pair<std::string, std::string>> *config);

#endif // _IS_READER_H_