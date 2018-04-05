#include <iostream>
#include <tinyxml2.h>
#include "RSBridgeRTPS.h"

#if defined(_WIN32) && defined (BUILD_SHARED_LIBS)
	#if defined (_MSC_VER)
		#pragma warning(disable: 4251)
	#endif
  #if defined(testlib_EXPORTS)
  	#define  USER_LIB_EXPORT __declspec(dllexport)
  #else
    #define  USER_LIB_EXPORT __declspec(dllimport)
  #endif
#else
  #define USER_LIB_EXPORT
#endif

RSBridgeRTPS* loadFastRTPSBridge(tinyxml2::XMLElement *bridge_element);

extern "C" RSBridge* USER_LIB_EXPORT createBridge(tinyxml2::XMLElement *bridge_element)
{
    return loadFastRTPSBridge(bridge_element);
}

tinyxml2::XMLElement* _assignNextElement(tinyxml2::XMLElement *element, std::string name){
    if (!element->FirstChildElement(name.c_str())){
        throw 0;
    }
    return element->FirstChildElement(name.c_str());
}

tinyxml2::XMLElement* _assignOptionalElement(tinyxml2::XMLElement *element, std::string name){
    return element->FirstChildElement(name.c_str());
}

RSBridgeRTPS* loadFastRTPSBridge(tinyxml2::XMLElement *bridge_element)
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

        current_element = _assignOptionalElement(subscriber_element, "partition");
        const char* input_partition = (current_element == nullptr) ? nullptr : current_element->GetText();

        current_element = _assignNextElement(publisher_element, "participant");
        const char* output_participant_name = current_element->GetText();
        current_element = _assignNextElement(publisher_element, "topic");
        const char* output_topic_name = current_element->GetText();
        current_element = _assignNextElement(publisher_element, "type");
        const char* output_type_name = current_element->GetText();

        current_element = _assignOptionalElement(publisher_element, "partition");
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

        RSBridgeRTPS *bridge = new RSBridgeRTPS(participant_publisher_params,
                            participant_subscriber_params,
                            publisher_params,
                            subscriber_params,
                            function_path);

        std::cout << "Created bridge Fast-RTPS between [" << input_topic_name << ":" << input_domain << "] and [" <<
            output_topic_name << ":" << output_domain << "]" << std::endl;

        return bridge;
    }
    catch (int e_code){
        std::cout << "Invalid configuration, skipping bridge " << e_code << std::endl;
    }
}
