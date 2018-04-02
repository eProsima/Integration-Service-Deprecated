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

#include "RSManager.h"
#include "NGSIv2Params.h"

RSManager::RSManager(std::string xml_file_path) : active(false)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xml_file_path.c_str());

    tinyxml2::XMLElement *bridge_element = doc.FirstChildElement("rs");

    if (!bridge_element)
    {
        std::cout << "Invalid config file" << std::endl;
        return;
    }

    for (auto child = bridge_element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
    {
        tinyxml2::XMLElement *current_element = _assignNextElement(child, "bridge_type");
        const char* bridge_type = current_element->GetText();
        if (strncmp(bridge_type, "fastrtps", 8) == 0 || strncmp(bridge_type, "ros2", 4) == 0)
        {
            LoadFastRTPSBridge(child);
        }
        else if (strncmp(bridge_type, "ngsiv2_to_fastrtps", 6) == 0)
        {
            LoadNGSIv2ToFastRTPSBridge(child);
        }
        else if (strncmp(bridge_type, "fastrtps_to_ngsiv2", 6) == 0)
        {
            LoadFastRTPSToNGSIv2Bridge(child);
        }
    }
}

void RSManager::LoadFastRTPSBridge(tinyxml2::XMLElement *bridge_element)
{
    try
    {
        tinyxml2::XMLElement *subscriber_element = bridge_element->FirstChildElement("subscriber");
        tinyxml2::XMLElement *publisher_element = bridge_element->FirstChildElement("publisher");
        if(!subscriber_element || !publisher_element)
        {
            throw 0;
        }

        tinyxml2::XMLElement *current_element;
        current_element = _assignNextElement(subscriber_element, "participant");
        const char* input_participant_name = current_element->GetText();
        current_element = _assignNextElement(subscriber_element, "topic");
        const char* input_topic_name = current_element->GetText();
        current_element = _assignNextElement(subscriber_element, "type");
        const char* input_type_name = current_element->GetText();
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(subscriber_element, "partition");
        }
        catch (...)
        {}
        const char* input_partition = (current_element == nullptr) ? nullptr : current_element->GetText();
        
        current_element = _assignNextElement(publisher_element, "participant");
        const char* output_participant_name = current_element->GetText();
        current_element = _assignNextElement(publisher_element, "topic");
        const char* output_topic_name = current_element->GetText();
        current_element = _assignNextElement(publisher_element, "type");
        const char* output_type_name = current_element->GetText();
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(publisher_element, "partition");
        }
        catch (...)
        {}
        const char* output_partition = (current_element == nullptr) ? nullptr : current_element->GetText();

        int input_domain = 0;
        current_element = _assignNextElement(subscriber_element, "domain");
        if(current_element->QueryIntText(&input_domain))
        {
            throw 0;
        }

        int output_domain = 0;
        current_element = _assignNextElement(publisher_element, "domain");
        if(current_element->QueryIntText(&output_domain))
        {
            throw 0;
        }

        const char* function_path;
        if (bridge_element->FirstChildElement("transformation"))
        {
            function_path = bridge_element->FirstChildElement("transformation")->GetText();
        }
        else
        {
            function_path = nullptr;
        }

        // Participant (publisher) configuration
        ParticipantAttributes participant_publisher_params;
        participant_publisher_params.rtps.builtin.domainId = output_domain;
        participant_publisher_params.rtps.builtin.leaseDuration = c_TimeInfinite;
        participant_publisher_params.rtps.setName(output_participant_name);

        // Participant (subscriber) configuration
        ParticipantAttributes participant_subscriber_params;
        participant_subscriber_params.rtps.builtin.domainId = input_domain;
        participant_subscriber_params.rtps.builtin.leaseDuration = c_TimeInfinite;
        participant_subscriber_params.rtps.setName(input_participant_name);

        // Publisher configuration
        PublisherAttributes publisher_params;
        publisher_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        publisher_params.topic.topicDataType = output_type_name;
        publisher_params.topic.topicName = output_topic_name;
        if (output_partition != nullptr)
        {
            publisher_params.qos.m_partition.push_back(output_partition);
        }

        // Subscriber configuration
        SubscriberAttributes subscriber_params;
        subscriber_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        subscriber_params.topic.topicKind = NO_KEY;
        subscriber_params.topic.topicDataType = input_type_name;
        subscriber_params.topic.topicName = input_topic_name;
        if (input_partition != nullptr)
        {
            subscriber_params.qos.m_partition.push_back(input_partition);
        }

        bridge.emplace_back(participant_publisher_params,
                            participant_subscriber_params,
                            publisher_params,
                            subscriber_params,
                            function_path);
        active = true;

        std::cout << "Created bridge Fast-RTPS between [" << input_topic_name << ":" << input_domain << "] and [" <<
            output_topic_name << ":" << output_domain << "]" << std::endl;
    }
    catch (int e_code){
        std::cout << "Invalid configuration, skipping bridge " << e_code << std::endl;
    }
}

void RSManager::LoadNGSIv2ToFastRTPSBridge(tinyxml2::XMLElement *bridge_element)
{
    try
    {
        tinyxml2::XMLElement *fastrtps_element = bridge_element->FirstChildElement("fastrtps");
        if (!fastrtps_element)
        {
            fastrtps_element = bridge_element->FirstChildElement("ros2");
        }
        tinyxml2::XMLElement *ngsiv2_element = bridge_element->FirstChildElement("ngsiv2");
        if(!fastrtps_element || !ngsiv2_element)
        {
            throw 0;
        }

        tinyxml2::XMLElement *current_element;
        current_element = _assignNextElement(fastrtps_element, "participant");
        const char* fastrtps_participant_name = current_element->GetText();
        current_element = _assignNextElement(fastrtps_element, "topic");
        const char* fastrtps_topic_name = current_element->GetText();
        current_element = _assignNextElement(fastrtps_element, "type");
        const char* fastrtps_type_name = current_element->GetText();
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(fastrtps_element, "partition");
        }
        catch (...)
        {}
        const char* fastrtps_partition = (current_element == nullptr) ? nullptr : current_element->GetText();
        
        current_element = _assignNextElement(ngsiv2_element, "participant");
        const char* ngsiv2_participant_name = current_element->GetText();
        current_element = _assignNextElement(ngsiv2_element, "id");
        const char* ngsiv2_id = current_element->GetText();
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(ngsiv2_element, "host");
        }
        catch (...)
        {}
        const char* ngsiv2_host = (current_element == nullptr) ? "localhost" : current_element->GetText();

        int fastrtps_domain = 0;
        current_element = _assignNextElement(fastrtps_element, "domain");
        if(current_element->QueryIntText(&fastrtps_domain))
        {
            throw 0;
        }
        
        int ngsiv2_port;
        current_element = _assignNextElement(ngsiv2_element, "port");
        if(current_element->QueryIntText(&ngsiv2_port))
        {
            ngsiv2_port = 1026;
        }
        
        // Subscription
        tinyxml2::XMLElement *ngsiv2_sub_element = _assignNextElement(ngsiv2_element, "subscription");
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(ngsiv2_sub_element, "type");
        }
        catch (...)
        {}
        const char* subscription_type = (current_element == nullptr) ? "" : current_element->GetText();
        
        const char* subscription_attrs = _assignNextElement(ngsiv2_sub_element, "attrs")->GetText();
        const char* subscription_expression = _assignNextElement(ngsiv2_sub_element, "expression")->GetText();
        const char* subscription_notifs = _assignNextElement(ngsiv2_sub_element, "notifs")->GetText();
        
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(ngsiv2_sub_element, "expiration");
        }
        catch (...)
        {}
        const char* subscription_expiration = (current_element == nullptr) ? "" : current_element->GetText();
        
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(ngsiv2_sub_element, "throttling");
        }
        catch (...)
        {}
        int subscription_throttling = -1;
        if (current_element != nullptr)
        {
            current_element->QueryIntText(&subscription_throttling);
        }
        
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(ngsiv2_sub_element, "description");
        }
        catch (...)
        {}
        const char* subscription_description = (current_element == nullptr) ? "" : current_element->GetText();

        const char* function_path;
        if (bridge_element->FirstChildElement("transformation"))
        {
            function_path = bridge_element->FirstChildElement("transformation")->GetText();
        }
        else
        {
            function_path = nullptr;
        }

        // Participant (publisher) configuration
        ParticipantAttributes participant_fastrtps_params;
        participant_fastrtps_params.rtps.builtin.domainId = fastrtps_domain;
        participant_fastrtps_params.rtps.builtin.leaseDuration = c_TimeInfinite;
        participant_fastrtps_params.rtps.setName(fastrtps_participant_name);

        // Publisher configuration
        PublisherAttributes publisher_params;
        publisher_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        publisher_params.topic.topicDataType = fastrtps_type_name;
        publisher_params.topic.topicName = fastrtps_topic_name;
        if (fastrtps_partition != nullptr)
        {
            publisher_params.qos.m_partition.push_back(fastrtps_partition);
        }
        
        NGSIv2Params participant_ngsiv2_params;
        participant_ngsiv2_params.name = ngsiv2_participant_name;
        participant_ngsiv2_params.host = ngsiv2_host;
        participant_ngsiv2_params.idPattern = ngsiv2_id;
        participant_ngsiv2_params.port = ngsiv2_port;
        
        NGSIv2SubscriptionParams participant_ngsiv2_subscription_param;
        participant_ngsiv2_subscription_param.idPattern = ngsiv2_id;
        participant_ngsiv2_subscription_param.type = subscription_type;
        participant_ngsiv2_subscription_param.attrs = subscription_attrs;
        participant_ngsiv2_subscription_param.expression = subscription_expression;
        participant_ngsiv2_subscription_param.notif = subscription_notifs;
        participant_ngsiv2_subscription_param.expiration = subscription_expiration;
        participant_ngsiv2_subscription_param.throttling = subscription_throttling;
        participant_ngsiv2_subscription_param.description = subscription_description;

        bridgeNGSIv2ToFastRTPS.emplace_back(participant_ngsiv2_params,
                            participant_fastrtps_params,
                            participant_ngsiv2_subscription_param,
                            publisher_params,
                            function_path);
        active = true;

        std::cout << "Created bridge NGSIv2-FastRTPS between [" << fastrtps_topic_name << ":" << fastrtps_domain << "] and [" <<
            ngsiv2_participant_name << ":" << ngsiv2_id << "]" << std::endl;
    }
    catch (int e_code){
        std::cout << "Invalid configuration, skipping bridge " << e_code << std::endl;
    }
}

void RSManager::LoadFastRTPSToNGSIv2Bridge(tinyxml2::XMLElement *bridge_element)
{
    try
    {
        tinyxml2::XMLElement *fastrtps_element = bridge_element->FirstChildElement("fastrtps");
        if (!fastrtps_element)
        {
            fastrtps_element = bridge_element->FirstChildElement("ros2");
        }
        tinyxml2::XMLElement *ngsiv2_element = bridge_element->FirstChildElement("ngsiv2");
        if(!fastrtps_element || !ngsiv2_element)
        {
            throw 0;
        }

        tinyxml2::XMLElement *current_element;
        current_element = _assignNextElement(fastrtps_element, "participant");
        const char* fastrtps_participant_name = current_element->GetText();
        current_element = _assignNextElement(fastrtps_element, "topic");
        const char* fastrtps_topic_name = current_element->GetText();
        current_element = _assignNextElement(fastrtps_element, "type");
        const char* fastrtps_type_name = current_element->GetText();
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(fastrtps_element, "partition");
        }
        catch (...)
        {}
        const char* fastrtps_partition = (current_element == nullptr) ? nullptr : current_element->GetText();
        
        current_element = _assignNextElement(ngsiv2_element, "participant");
        const char* ngsiv2_participant_name = current_element->GetText();
        current_element = _assignNextElement(ngsiv2_element, "id");
        const char* ngsiv2_id = current_element->GetText();
        try
        {
            current_element = nullptr;
            current_element = _assignNextElement(ngsiv2_element, "host");
        }
        catch (...)
        {}
        const char* ngsiv2_host = (current_element == nullptr) ? "localhost" : current_element->GetText();

        int fastrtps_domain = 0;
        current_element = _assignNextElement(fastrtps_element, "domain");
        if(current_element->QueryIntText(&fastrtps_domain))
        {
            throw 0;
        }
        
        int ngsiv2_port;
        current_element = _assignNextElement(ngsiv2_element, "port");
        if(current_element->QueryIntText(&ngsiv2_port))
        {
            ngsiv2_port = 1026;
        }


        const char* function_path;
        if (bridge_element->FirstChildElement("transformation"))
        {
            function_path = bridge_element->FirstChildElement("transformation")->GetText();
        }
        else
        {
            function_path = nullptr;
        }

        // Participant (publisher) configuration
        ParticipantAttributes participant_fastrtps_params;
        participant_fastrtps_params.rtps.builtin.domainId = fastrtps_domain;
        participant_fastrtps_params.rtps.builtin.leaseDuration = c_TimeInfinite;
        participant_fastrtps_params.rtps.setName(fastrtps_participant_name);

        // Subscriber configuration
        SubscriberAttributes subscriber_params;
        subscriber_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        subscriber_params.topic.topicKind = NO_KEY;
        subscriber_params.topic.topicDataType = fastrtps_type_name;
        subscriber_params.topic.topicName = fastrtps_topic_name;
        if (fastrtps_partition != nullptr)
        {
            subscriber_params.qos.m_partition.push_back(fastrtps_partition);
        }
        
        NGSIv2Params participant_ngsiv2_params;
        participant_ngsiv2_params.name = ngsiv2_participant_name;
        participant_ngsiv2_params.host = ngsiv2_host;
        participant_ngsiv2_params.idPattern = ngsiv2_id;
        participant_ngsiv2_params.port = ngsiv2_port;

        bridgeFastRTPSToNGSIv2.emplace_back(participant_fastrtps_params,
                            participant_ngsiv2_params,
                            subscriber_params,
                            function_path);
        active = true;

        std::cout << "Created bridge FastRTPS-NGSIv2 between [" << fastrtps_topic_name << ":" << fastrtps_domain << "] and [" <<
            ngsiv2_participant_name << ":" << ngsiv2_id << "]" << std::endl;
    }
    catch (int e_code){
        std::cout << "Invalid configuration, skipping bridge " << e_code << std::endl;
    }
}

tinyxml2::XMLElement* RSManager::_assignNextElement(tinyxml2::XMLElement *element, std::string name){
    if (!element->FirstChildElement(name.c_str())){
        throw 0;
    }
    return element->FirstChildElement(name.c_str());
}

bool RSManager::isActive(){
    return active;
}
