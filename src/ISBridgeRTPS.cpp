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

ISBridgeRTPS::ISBridgeRTPS(RTPSPublisher *pub, RTPSListener *sub, const char *file_path)
{
    mp_publisher = nullptr;
    ms_subscriber = nullptr;
    sub->setTransformation(file_path, "transform");
    sub->setPublisher(pub);
    rtps_publisher = pub;
    rtps_subscriber = sub;
}

ISBridgeRTPS::~ISBridgeRTPS(){
    delete ms_subscriber;
    delete mp_publisher;
    delete rtps_subscriber;
    delete rtps_publisher;
}

void RTPSListener::onSubscriptionMatched(Subscriber* sub, MatchingInfo& info){
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

RTPSListener::RTPSListener() : handle(nullptr), user_transformation(nullptr)
{
}

void RTPSListener::setTransformation(const char* file_path, const char* transformation_name)
{
    if(file_path){
        handle = eProsimaLoadLibrary(file_path);
        user_transformation = (userf_t)eProsimaGetProcAddress(handle, transformation_name);
    }
}

RTPSPublisher::~RTPSPublisher()
{
    if(mp_participant != nullptr) Domain::removeParticipant(mp_participant);
}

RTPSListener::~RTPSListener()
{
    if(handle) eProsimaCloseLibrary(handle);
    if(ms_participant != nullptr) Domain::removeParticipant(ms_participant);
}

void ISBridgeRTPS::onTerminate()
{
    // Don't need to do anything here
}

void RTPSListener::onNewDataMessage(Subscriber* sub){
    onDataReceived(sub); // No need to call, work could be done here, but as example is fine.
}

bool RTPSListener::onDataReceived(void* data)
{
    Subscriber* sub = (Subscriber*)data;
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
            if (listener_publisher)
            {
                return listener_publisher->publish(&serialized_output);
            }
        }
    }
    return false;
}

RTPSListener* RTPSListener::configureRTPSSubscriber(void *configuration)
{
    tinyxml2::XMLElement *subscriber_element = (tinyxml2::XMLElement*)configuration;

    tinyxml2::XMLElement *current_element;
    current_element = _assignNextElement(subscriber_element, "participant");
    const char* input_participant_name = current_element->GetText();
    current_element = _assignNextElement(subscriber_element, "topic");
    const char* input_topic_name = current_element->GetText();
    current_element = _assignNextElement(subscriber_element, "type");
    const char* input_type_name = current_element->GetText();

    current_element = _assignOptionalElement(subscriber_element, "partition");
    const char* input_partition = (current_element == nullptr) ? nullptr : current_element->GetText();

    int input_domain = 0;
    current_element = _assignNextElement(subscriber_element, "domain");
    if(current_element->QueryIntText(&input_domain))
    {
        return nullptr;
    }

    // Participant (subscriber) configuration
    ParticipantAttributes par_sub_params;
    par_sub_params.rtps.builtin.domainId = input_domain;
    par_sub_params.rtps.builtin.leaseDuration = c_TimeInfinite;
    par_sub_params.rtps.setName(input_participant_name);

    // Subscriber configuration
    SubscriberAttributes sub_params;
    sub_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    sub_params.topic.topicKind = NO_KEY;
    sub_params.topic.topicDataType = input_type_name;
    sub_params.topic.topicName = input_topic_name;
    if (input_partition != nullptr)
    {
        sub_params.qos.m_partition.push_back(input_partition);
    }

    RTPSListener* listener = new RTPSListener();
    listener->ms_participant = Domain::createParticipant(par_sub_params);
    if(listener->ms_participant == nullptr)
    {
        delete listener;
        return nullptr;
    }

    listener->input_type = new GenericPubSubType();
    listener->input_type->setName(sub_params.topic.topicDataType.c_str());
    Domain::registerType(listener->ms_participant,(TopicDataType*) listener->input_type);

    // Create Subscriber
    listener->ms_subscriber = Domain::createSubscriber(listener->ms_participant, sub_params,
                                                       (SubscriberListener*)listener);
    if(listener->ms_subscriber == nullptr)
    {
        delete listener;
        return nullptr;
    }

    // Set name
    std::string topic_name = input_topic_name;
    std::string domain_name = std::to_string(input_domain);
    listener->name = topic_name + ":" + domain_name;

    return listener;
}

RTPSPublisher* RTPSPublisher::configureRTPSPublisher(void *configuration)
{
    tinyxml2::XMLElement *publisher_element = (tinyxml2::XMLElement*)configuration;
    tinyxml2::XMLElement *current_element;

    current_element = _assignNextElement(publisher_element, "participant");
    const char* output_participant_name = current_element->GetText();
    current_element = _assignNextElement(publisher_element, "topic");
    const char* output_topic_name = current_element->GetText();
    current_element = _assignNextElement(publisher_element, "type");
    const char* output_type_name = current_element->GetText();

    current_element = _assignOptionalElement(publisher_element, "partition");
    const char* output_partition = (current_element == nullptr) ? nullptr : current_element->GetText();

    int output_domain = 0;
    current_element = _assignNextElement(publisher_element, "domain");
    if(current_element->QueryIntText(&output_domain))
    {
        return nullptr;
    }

    // Participant (publisher) configuration
    ParticipantAttributes par_pub_params;
    par_pub_params.rtps.builtin.domainId = output_domain;
    par_pub_params.rtps.builtin.leaseDuration = c_TimeInfinite;
    par_pub_params.rtps.setName(output_participant_name);

    // Publisher configuration
    PublisherAttributes pub_params;
    pub_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    pub_params.topic.topicKind = NO_KEY;
    pub_params.topic.topicDataType = output_type_name;
    pub_params.topic.topicName = output_topic_name;
    if (output_partition != nullptr)
    {
        pub_params.qos.m_partition.push_back(output_partition);
    }

    RTPSPublisher* publisher = new RTPSPublisher();

    // Create RTPSParticipant
    publisher->mp_participant = Domain::createParticipant(par_pub_params);
    if(publisher->mp_participant == nullptr)
    {
        delete publisher;
        return nullptr;
    }

    //Register types
    publisher->output_type = new GenericPubSubType();
    publisher->output_type->setName(pub_params.topic.topicDataType.c_str());
    Domain::registerType(publisher->mp_participant,(TopicDataType*) publisher->output_type);

    //Create publisher
    publisher->mp_publisher = Domain::createPublisher(publisher->mp_participant, pub_params,
                                      (PublisherListener*)publisher);
    if(publisher->mp_publisher == nullptr)
    {
        delete publisher;
        return nullptr;
    }

    // Set name
    std::string topic_name = output_topic_name;
    std::string domain_name = std::to_string(output_domain);
    publisher->name = topic_name + ":" + domain_name;

    return publisher;
}

ISBridge* ISBridgeRTPS::configureBridge(void *configuration)
{
    try
    {
        tinyxml2::XMLElement *bridge_element = (tinyxml2::XMLElement*)configuration;
        tinyxml2::XMLElement *subscriber_element = bridge_element->FirstChildElement("subscriber");
        tinyxml2::XMLElement *publisher_element = bridge_element->FirstChildElement("publisher");
        if(!subscriber_element || !publisher_element)
        {
            throw 0;
        }

        RTPSListener* listener = RTPSListener::configureRTPSSubscriber(subscriber_element);
        RTPSPublisher* publisher = RTPSPublisher::configureRTPSPublisher(publisher_element);

        if (!listener)
        {
            std::cout << "Error loading RTPS listener configuration. " << std::endl;
            return nullptr;
        }
        else if (!publisher)
        {
            std::cout << "Error loading RTPS publisher configuration. " << std::endl;
            return nullptr;
        }
        else
        {
            const char* function_path;
            if (bridge_element->FirstChildElement("transformation"))
            {
                function_path = bridge_element->FirstChildElement("transformation")->GetText();
            }
            else
            {
                function_path = nullptr;
            }

            ISBridgeRTPS *bridge = new ISBridgeRTPS(publisher, listener, function_path);

            std::cout << "Created bridge Fast-RTPS between [" << listener->name << "] and [" <<
                publisher->name << "]" << std::endl;

            return bridge;
        }
    }
    catch (int e_code){
        std::cout << "Invalid configuration, skipping bridge " << e_code << std::endl;
    }
    return nullptr;
}
