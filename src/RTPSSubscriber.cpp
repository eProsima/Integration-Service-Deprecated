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

#include <fastrtps/types/DynamicPubSubType.h>
#include "RTPSSubscriber.h"
#include "GenericPubSubTypes.h"
#include "log/ISLog.h"

using eprosima::fastcdr::Cdr;

RTPSSubscriber::RTPSSubscriber(const std::string &name)
    : ISReader(name)
    , ms_participant(nullptr)
    , ms_subscriber(nullptr)
    , m_dynData(nullptr)
    , m_buffer(nullptr)
    , m_payload(nullptr)
{
}

RTPSSubscriber::~RTPSSubscriber()
{
    //if(handle) eProsimaCloseLibrary(handle);

    // Participants are deleted from the ISManager.
    if (m_dynData != nullptr)
    {
        DynamicDataFactory::GetInstance()->DeleteData(m_dynData);
    }
    if (m_buffer != nullptr)
    {
        input_type->deleteData(m_buffer);
    }
    delete m_payload;
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
    if (dynamic_cast<GenericPubSubType*>(input_type) != nullptr)
    {
        if (m_payload == nullptr)
        {
            m_payload = new SerializedPayload_t();
        }

        bool taken = sub->takeNextData(m_payload, &m_info);

        if (taken && m_info.sampleKind == ALIVE)
        {
            on_received_data(m_payload);
        }
    }
    else if (dynamic_cast<DynamicPubSubType*>(input_type) != nullptr)
    {
        DynamicPubSubType *pst = dynamic_cast<DynamicPubSubType*>(input_type);

        if (m_dynData == nullptr)
        {
            m_dynData = DynamicDataFactory::GetInstance()->CreateData(pst->GetDynamicType());
        }
        else
        {
            m_dynData->ClearAllValues();
        }
        bool taken = sub->takeNextData(m_dynData, &m_info);

        if (taken && m_info.sampleKind == ALIVE)
        {
            on_received_data(m_dynData);
        }
    }
    else
    {
        // Provided type
        if (m_buffer == nullptr)
        {
            m_buffer = input_type->createData();
        }

        bool taken = sub->takeNextData(m_buffer, &m_info);

        if (m_payload == nullptr)
        {
            m_payload = new SerializedPayload_t(input_type->m_typeSize);
        }

        input_type->serialize(m_buffer, m_payload);

        if (taken && m_info.sampleKind == ALIVE)
        {
            on_received_data(m_payload);
        }
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