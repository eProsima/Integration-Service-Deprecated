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
#include "RTPSPublisher.h"
#include "log/ISLog.h"


RTPSPublisher::RTPSPublisher(const std::string &name)
    : ISPublisher(name)
    , m_buffer(nullptr)
{
}

RTPSPublisher::~RTPSPublisher()
{
    // Participants are deleted from the ISManager.
    if (m_buffer != nullptr)
    {
        output_type->deleteData(m_buffer);
    }
    mp_participant = nullptr;
}

void RTPSPublisher::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        LOG_INFO("Publisher matched");
    }
    else
    {
        LOG_INFO("Publisher unmatched");
    }
}

bool RTPSPublisher::publish(SerializedPayload_t *data)
{
    if (dynamic_cast<GenericPubSubType*>(output_type) != nullptr)
    {
        return mp_publisher->write(data);
    }
    else
    {
        std::unique_lock<std::mutex> scopeLock(m_bufferMutex);
        if (m_buffer == nullptr)
        {
            m_buffer = output_type->createData();
        }
        output_type->deserialize(data, m_buffer);
        bool result = mp_publisher->write(m_buffer);
        return result;
    }
}

bool RTPSPublisher::publish(DynamicData *data)
{
    return mp_publisher->write(data);
}

void RTPSPublisher::setParticipant(Participant* part)
{
    mp_participant = part;
}

bool RTPSPublisher::hasParticipant()
{
    return mp_participant != nullptr;
}

Participant* RTPSPublisher::getParticipant()
{
    return mp_participant;
}

bool RTPSPublisher::hasRTPSPublisher()
{
    return mp_publisher != nullptr;
}

void RTPSPublisher::setRTPSPublisher(Publisher* pub)
{
    mp_publisher = pub;
}
