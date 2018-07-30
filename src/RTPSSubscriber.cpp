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

#include <fastrtps/Domain.h>
#include <fastcdr/Cdr.h>
#include "RTPSSubscriber.h"
#include "GenericPubSubTypes.h"
#include "log/ISLog.h"

RTPSSubscriber::RTPSSubscriber(const std::string &name)
    : ISSubscriber(name)
    , ms_participant(nullptr)
    , ms_subscriber(nullptr)
{
}

RTPSSubscriber::~RTPSSubscriber()
{
    //if(handle) eProsimaCloseLibrary(handle);

    // Participants are deleted from the ISManager.
    ms_participant = nullptr;
}

void RTPSSubscriber::onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        LOG_INFO("Subscriber matched");
    }
    else
    {
        LOG_INFO("Subscriber unmatched");
    }
}

void RTPSSubscriber::onNewDataMessage(Subscriber* sub)
{
    SerializedPayload_t serialized_input(input_type->m_typeSize);

    bool taken = sub->takeNextSerializedPayload(&serialized_input, &m_info);

    if(taken && m_info.sampleKind == ALIVE)
    {
        on_received_data(&serialized_input);
    }
}

void RTPSSubscriber::setParticipant(Participant* part)
{
    ms_participant = part;
}

void RTPSSubscriber::setRTPSSubscriber(Subscriber* sub)
{
    ms_subscriber = sub;
}

bool RTPSSubscriber::hasParticipant()
{
    return ms_participant != nullptr;
}

bool RTPSSubscriber::hasRTPSSubscriber()
{
    return ms_subscriber != nullptr;
}

Participant* RTPSSubscriber::getParticipant()
{
    return ms_participant;
}
