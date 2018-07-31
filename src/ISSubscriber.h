#ifndef _IS_SUBSCRIBER_H_
#define _IS_SUBSCRIBER_H_

#include "ISBridge.h"

class ISBridge;

/** Base class for subscribers. Must know how to read from the origin protocol */
class ISSubscriber : public ISBaseClass
{
protected:
    std::vector<ISBridge*> mv_bridges;
public:
    ISSubscriber(const std::string &name) : ISBaseClass(name) { };
    virtual ~ISSubscriber() = default;
    virtual void addBridge(ISBridge* bridge){
        mv_bridges.push_back(bridge);
    }
    virtual void on_received_data(SerializedPayload_t* payload) = 0;

    // Forbid copy
    ISSubscriber(const ISSubscriber&) = delete;
    ISSubscriber& operator=(const ISSubscriber&) = delete;
};

typedef ISSubscriber* (*createSubf_t)(ISBridge *bridge, const char* name,
                                      const std::vector<std::pair<std::string, std::string>> *config);

#endif // _IS_SUBSCRIBER_H_