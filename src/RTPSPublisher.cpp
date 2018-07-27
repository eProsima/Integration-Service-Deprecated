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


RTPSPublisher::RTPSPublisher(const std::string &name) : ISPublisher(name)
{
}

RTPSPublisher::~RTPSPublisher()
{
    if(mp_participant != nullptr) Domain::removeParticipant(mp_participant);
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

bool RTPSPublisher::publish(SerializedPayload_t * data)
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
