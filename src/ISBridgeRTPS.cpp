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


#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/Domain.h>

#include "ISBridgeRTPS.h"
#include "GenericPubSubTypes.h"
#include "xmlUtils.h"

void RTPSListener::onSubscriptionMatched(Subscriber* sub, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

void RTPSPublisher::onPublicationMatched(Publisher* pub, MatchingInfo& info)
{
    if (info.status == MATCHED_MATCHING)
    {
        std::cout << "Publisher matched" << std::endl;
    }
    else
    {
        std::cout << "Publisher unmatched" << std::endl;
    }
}

RTPSPublisher::~RTPSPublisher()
{
    if(mp_participant != nullptr) Domain::removeParticipant(mp_participant);
}


RTPSListener::~RTPSListener()
{
    //if(handle) eProsimaCloseLibrary(handle);
    if(ms_participant != nullptr) Domain::removeParticipant(ms_participant);
}

void RTPSListener::onNewDataMessage(Subscriber* sub)
{
    SerializedPayload_t serialized_input;
    if(sub->takeNextData(&serialized_input, &m_info))
    {
        if(m_info.sampleKind == ALIVE)
        {
            on_received_data(&serialized_input);
            /*
            for (ISBridge* bridge : mv_bridges)
            {
                bridge->on_received_data(this, &serialized_input);
            }
            */
        }
    }
}
