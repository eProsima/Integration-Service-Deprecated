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

class RTPSPublisher : public PublisherListener, public ISPublisher
{
private:
    Publisher *mp_publisher;
    Participant *mp_participant;
public:
    GenericPubSubType *output_type;
    //RTPSPublisher() {}
    RTPSPublisher(const std::string &name) : ISPublisher(name) {}
    virtual bool publish(void * data) override
    {
        return mp_publisher->write(data);
    }
    ~RTPSPublisher() override;
    void onPublicationMatched(Publisher* pub, MatchingInfo& info) override;
    void setParticipant(Participant* part) { mp_participant = part; }
    bool hasParticipant() { return mp_participant != nullptr; }
    Participant* getParticipant() { return mp_participant; }
    bool hasRTPSPublisher() { return mp_publisher != nullptr; }
    void setRTPSPublisher(Publisher* pub) { mp_publisher = pub; }
};

class RTPSListener : public SubscriberListener, public ISSubscriber
{
private:
    SampleInfo_t m_info;
    Participant *ms_participant;
    Subscriber *ms_subscriber;
public:
    GenericPubSubType *input_type;
    //RTPSListener();
    RTPSListener(const std::string &name) : ISSubscriber(name), ms_participant(nullptr), ms_subscriber(nullptr) {}
    ~RTPSListener() override;
    void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info) override;
    void onNewDataMessage(Subscriber* sub) override;
    void setParticipant(Participant* part) { ms_participant = part; }
    void setRTPSSubscriber(Subscriber* sub) { ms_subscriber = sub; }
    bool hasParticipant() { return ms_participant != nullptr; }
    bool hasRTPSSubscriber() { return ms_subscriber != nullptr; }
    Participant* getParticipant() { return ms_participant; }
};

class ISBridgeRTPS : public ISBridge
{
public:
    ISBridgeRTPS(const std::string &name) : ISBridge(name) {}
    virtual ~ISBridgeRTPS() {};
};

#endif // _Header__SUBSCRIBER_H_
