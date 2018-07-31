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


#ifndef _ISBRIDGERTPS_SUBSCRIBER_H_
#define _ISBRIDGERTPS_SUBSCRIBER_H_

#include <string>

#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/TopicDataType.h>

#include "ISBridge.h"
#include "dynamicload/dynamicload.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class RTPSSubscriber : public SubscriberListener, public ISSubscriber
{
private:
    SampleInfo_t m_info;
    Participant *ms_participant;
    Subscriber *ms_subscriber;
public:
    TopicDataType *input_type;
    RTPSSubscriber(const std::string &name);
    ~RTPSSubscriber() override;
    void onSubscriptionMatched(Subscriber* sub, MatchingInfo& info) override;
    void onNewDataMessage(Subscriber* sub) override;
    void setParticipant(Participant* part);
    void setRTPSSubscriber(Subscriber* sub);
    bool hasParticipant();
    bool hasRTPSSubscriber();
    Participant* getParticipant();
    Subscriber* getRTPSSubscriber() { return ms_subscriber; }
};

#endif // _Header__SUBSCRIBER_H_
