// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef _ISBRIDGERTPS_H_
#define _ISBRIDGERTPS_H_

#include <string>

#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include <fastrtps/fastrtps_fwd.h>

#include "ISBridge.h"
#include "GenericPubSubTypes.h"
#include "dynamicload/dynamicload.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

typedef void (*userf_t)(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output);

class RTPSPublisher : public ISPublisher
{
private:
    Publisher *mp_publisher;
    Participant *mp_participant;
    GenericPubSubType *output_type;
public:
    RTPSPublisher() {}
    virtual bool publish(void * data) override
    {
        return mp_publisher->write(data);
    }
    ~RTPSPublisher() override;
    static RTPSPublisher* configureRTPSPublisher(void* configuration);
    std::string name;
};

class RTPSListener : public SubscriberListener, public ISSubscriber
{
private:
    userf_t user_transformation;
    void *handle;
    char *error;

    SampleInfo_t m_info;
    int n_matched;
    int n_msg;
    Participant *ms_participant;
    GenericPubSubType *input_type;
    Subscriber *ms_subscriber;
public:
    RTPSListener();
    ~RTPSListener() override;
    void setTransformation(const char* file_path);
    void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
    void onNewDataMessage(Subscriber* sub);
    virtual bool onDataReceived(void * data) override;
    static RTPSListener* configureRTPSSubscriber(void* configuration);
    std::string name;
};

class ISBridgeRTPS : public ISBridge
{
public:
    ISBridgeRTPS(RTPSPublisher *publisher, RTPSListener *listener, const char* file_path);
    virtual ~ISBridgeRTPS();
    void onTerminate() override;
    static ISBridge* configureBridge(void* configuration);
};

#endif // _Header__SUBSCRIBER_H_
