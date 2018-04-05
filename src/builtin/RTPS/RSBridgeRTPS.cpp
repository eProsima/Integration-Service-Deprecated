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
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/attributes/PublisherAttributes.h"

#include <fastrtps/Domain.h>

#include "RSBridgeRTPS.h"
#include "../../GenericPubSubTypes.h"

RSBridgeRTPS::RSBridgeRTPS(ParticipantAttributes par_pub_params,
                    ParticipantAttributes par_sub_params,
                    PublisherAttributes pub_params,
                    SubscriberAttributes sub_params,
                    const char *file_path
                ) : mp_participant(nullptr), ms_participant(nullptr),
                    ms_subscriber(nullptr),	mp_publisher(nullptr),
                    input_type(nullptr), output_type(nullptr),
                    m_listener(file_path)
{
    // Create RTPSParticipant
    mp_participant = Domain::createParticipant(par_pub_params);
    if(mp_participant == nullptr) return;

    // Create RTPSParticipant
    ms_participant = Domain::createParticipant(par_sub_params);
    if(ms_participant == nullptr) return;

    //Register types
    input_type = new GenericPubSubType();
    input_type->setName(sub_params.topic.topicDataType.c_str());
    Domain::registerType(ms_participant,(TopicDataType*) input_type);
    output_type = new GenericPubSubType();
    output_type->setName(pub_params.topic.topicDataType.c_str());
    Domain::registerType(mp_participant,(TopicDataType*) output_type);

    //Create publisher
    mp_publisher = Domain::createPublisher(mp_participant,pub_params,nullptr);
    if(mp_publisher == nullptr) return;
    m_listener.setPublisher(mp_publisher);

    // Create Subscriber
    ms_subscriber = Domain::createSubscriber(ms_participant,sub_params,(SubscriberListener*)&m_listener);
    if(ms_subscriber == nullptr) return;
}

RSBridgeRTPS::~RSBridgeRTPS(){
    if(mp_participant != nullptr) Domain::removeParticipant(mp_participant);
    if(ms_participant != nullptr) Domain::removeParticipant(ms_participant);
}

void RSBridgeRTPS::SubListener::onSubscriptionMatched(Subscriber* sub,MatchingInfo& info){
    if (info.status == MATCHED_MATCHING)
    {
        n_matched++;
        std::cout << "Subscriber matched" << std::endl;
    }
    else
    {
        n_matched--;
        std::cout << "Subscriber unmatched" << std::endl;
    }
}

RSBridgeRTPS::SubListener::SubListener(const char* file_path) : handle(nullptr), user_transformation(nullptr){
    if(file_path){
        handle = eProsimaLoadLibrary(file_path);
        user_transformation = (userf_t)eProsimaGetProcAddress(handle, "transform");
    }
}

RSBridgeRTPS::SubListener::~SubListener(){
    if(handle) eProsimaCloseLibrary(handle);
}

void RSBridgeRTPS::onTerminate()
{
    // Don't need to do anything here
}

void RSBridgeRTPS::SubListener::onNewDataMessage(Subscriber* sub){
    SerializedPayload_t serialized_input;
    SerializedPayload_t serialized_output;
    if(sub->takeNextData(&serialized_input, &m_info)){
        if(m_info.sampleKind == ALIVE){
            if(user_transformation){
                user_transformation(&serialized_input, &serialized_output);
            }
            else{
                serialized_output.copy(&serialized_input, false);
            }
            listener_publisher->write(&serialized_output);
        }
    }
}
