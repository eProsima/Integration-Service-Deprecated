#ifndef _IS_WRITER_H_
#define _IS_WRITER_H_

#include "ISBridge.h"
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>

class ISBridge;

/** Base class for writers. They must know how to write into the destination protocol */
class ISWriter : public ISBaseClass
{
protected:
    ISBridge *mb_bridge;
public:
    eprosima::fastrtps::TopicDataType *output_type;
    ISWriter(const std::string &name) : ISBaseClass(name), mb_bridge(nullptr), output_type(nullptr) { };
    virtual ~ISWriter() = default;

    virtual bool write(eprosima::fastrtps::rtps::SerializedPayload_t* /*data*/) = 0;
    virtual bool write(eprosima::fastrtps::types::DynamicData* /*data*/) = 0;

    virtual ISBridge* setBridge(ISBridge *);

    // Forbid copy
    ISWriter(const ISWriter&) = delete;
    ISWriter& operator=(const ISWriter&) = delete;
};

typedef ISWriter* (*createPubf_t)(ISBridge *bridge, const char* name,
                                    const std::vector<std::pair<std::string, std::string>> *config);

#endif //_IS_WRITER_H_