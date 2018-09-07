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

#include "ISManager.h"
#include "RTPSBridge.h"
#include "RTPSPublisher.h"
#include "RTPSSubscriber.h"
#include "xmlUtils.h"
#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

// String literals
static const std::string s_sIS("is");
static const std::string s_sProfiles("profiles");
static const std::string s_sParticipant("participant");
static const std::string s_sParticipants("participants");
static const std::string s_sBridge("bridge");
static const std::string s_sConnector("connector");
static const std::string s_sName("name");
static const std::string s_sAttributes("attributes");
static const std::string s_sDomain("domain");
static const std::string s_sSubscriber("subscriber");
static const std::string s_sPublisher("publisher");
static const std::string s_sTopic("topic");
static const std::string s_sType("type");
static const std::string s_sTypes("types");
static const std::string s_sPartition("partition");
static const std::string s_sLibrary("library");
static const std::string s_sProperties("properties");
static const std::string s_sProperty("property");
static const std::string s_sFuncCreateBridge("create_bridge");
static const std::string s_sFuncCreateSubscriber("create_subscriber");
static const std::string s_sFuncCreatePublisher("create_publisher");
static const std::string s_sTransformation("transformation");
static const std::string s_sParticipantName("participant_name");
static const std::string s_sBridgeName("bridge_name");
static const std::string s_sSubscriberName("subscriber_name");
static const std::string s_sPublisherName("publisher_name");
static const std::string s_sFile("file");
static const std::string s_sFunction("function");
static const std::string s_sValue("value");
// TCP parameters
static const std::string s_sProtocol("protocol");
static const std::string s_sRemoteAddress("remoteAddesss");
static const std::string s_sRemotePort("remotePort");
static const std::string s_sListeningPort("listeningPort");
static const std::string s_sLogicalInitialPeerPort("logicalInitialPeerPort");
static const std::string s_sLogicalMetadataPort("logicalMetadataPort");
static const std::string s_sLogicalUserPort("logicalUserPort");
// Topic types libraries
static const std::string s_sTopicTypes("topic_types");
static const std::string s_sTypesLibrary("types_library");


ISManager::ISManager(const std::string &xml_file_path)
    : active(false)
{
    //Log::SetVerbosity(Log::Warning);
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xml_file_path.c_str());

    tinyxml2::XMLElement *bridge_element = doc.FirstChildElement(s_sIS.c_str());
    if (!bridge_element)
    {
        LOG("Invalid config file");
        return;
    }

    for (auto child = doc.FirstChildElement(s_sIS.c_str());
        child != nullptr; child = child->NextSiblingElement(s_sIS.c_str()))
    {

        tinyxml2::XMLElement *types = child->FirstChildElement(s_sTypes.c_str());
        if(types)
        {
            //loadDynamicTypes(types);
            loadDynamicTypes(child);
        }

        tinyxml2::XMLElement *topic_types = child->FirstChildElement(s_sTopicTypes.c_str());
        if(topic_types)
        {
            loadTopicTypes(topic_types);
        }

        tinyxml2::XMLElement *profiles = child->FirstChildElement(s_sProfiles.c_str());
        if(profiles)
        {
            loadProfiles(profiles);
        }
        else
        {
            LOG_ERROR("No profiles found!");
            return;
        }

        /*
        tinyxml2::XMLElement *participant = child->FirstChildElement(s_sParticipant.c_str());
        while(participant)
        {
            loadParticipant(participant);
            participant = participant->NextSiblingElement(s_sParticipant.c_str());
        }
        */

        tinyxml2::XMLElement *bridge = child->FirstChildElement(s_sBridge.c_str());
        while(bridge)
        {
            loadBridge(bridge);
            bridge = bridge->NextSiblingElement(s_sBridge.c_str());
        }

        tinyxml2::XMLElement *connector = child->FirstChildElement(s_sConnector.c_str());
        while(connector)
        {
            loadConnector(connector);
            connector = connector->NextSiblingElement(s_sConnector.c_str());
        }
    }

    if (bridges.size() > 0)
    {
        active = true;
    }
}

void ISManager::addBridge(ISBridge* b)
{
    //bridge.emplace_back(b);
    bridges[b->getName()] = b;
    active = true;
}

void ISManager::addSubscriber(ISSubscriber* s)
{
    subscribers[s->getName()] = s;
}

void ISManager::addPublisher(ISPublisher* p)
{
    publishers[p->getName()] = p;
}

void ISManager::addSubscriber(const std::string &part_name, ISSubscriber* s)
{
    subscribers[getEndPointName(part_name, s->getName())] = s;
}

void ISManager::addPublisher(const std::string &part_name, ISPublisher* p)
{
    publishers[getEndPointName(part_name, p->getName())] = p;
}

void ISManager::loadProfiles(tinyxml2::XMLElement *profiles)
{
    xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLProfiles(*profiles);

    if (ret == xmlparser::XMLP_ret::XML_OK)
    {
        LOG_INFO("Profiles parsed successfully.");
    }
    else
    {
        LOG_ERROR("Error parsing profiles!");
    }
}

void ISManager::loadDynamicTypes(tinyxml2::XMLElement *types)
{
    xmlparser::XMLP_ret ret = xmlparser::XMLProfileManager::loadXMLDynamicTypes(*types);

    if (ret == xmlparser::XMLP_ret::XML_OK)
    {
        LOG_INFO("Dynamic Types parsed successfully.");
    }
    else
    {
        LOG_ERROR("Error parsing dynamic types!");
    }
}

void ISManager::loadTopicTypes(tinyxml2::XMLElement *topic_types_element)
{
    try
    {
        tinyxml2::XMLElement *type = topic_types_element->FirstChildElement(s_sType.c_str());
        if (!type)
        {
            LOG("No types found in topic_types");
            throw 0;
        }

        while (type)
        {
            const char* type_name = type->Attribute(s_sName.c_str());

            tinyxml2::XMLElement *element = _assignOptionalElement(type, s_sLibrary);
            if (element != nullptr)
            {
                // Has his own library
                const char* lib = (element->GetText() == nullptr) ? nullptr : element->GetText();
                if (lib != nullptr)
                {
                    void* handle = getLibraryHandle(lib);
                    typef_t function = (typef_t)eProsimaGetProcAddress(handle, "GetTopicType");
                    typesLibs[type_name] = function;
                }
            }

            element = _assignOptionalElement(type, s_sParticipants);
            if (element != nullptr)
            {
                for (tinyxml2::XMLElement *part = element->FirstChildElement(s_sParticipant.c_str());
                    part != nullptr; part = part->NextSiblingElement(s_sParticipant.c_str()) )
                {
                    const char* partName = part->Attribute(s_sName.c_str());
                    to_register_types.emplace_back(partName, type_name);
                }
            }

            type = type->NextSiblingElement(s_sType.c_str());
        }

        tinyxml2::XMLElement *typesLib = topic_types_element->FirstChildElement(s_sTypesLibrary.c_str());
        while (typesLib)
        {
            const char* lib = (typesLib->GetText() == nullptr) ? nullptr : typesLib->GetText();
            if (lib != nullptr)
            {
                void* handle = getLibraryHandle(lib);
                typef_t function = (typef_t)eProsimaGetProcAddress(handle, "GetTopicType");
                defaultTypesLibs.emplace_back(function);
                typesLib = typesLib->NextSiblingElement(s_sTypesLibrary.c_str());
            }
        }

        for(auto &pair : to_register_types)
        {
            TopicDataType* type = getTopicDataType(pair.second);
            data_types[pair] = type;
        }
    }
    catch (int e_code)
    {
        LOG_ERROR("Error ocurred while loading topic types " << e_code);
    }
}

TopicDataType* ISManager::getTopicDataType(const std::string &name)
{
    TopicDataType* type = nullptr;

    // Try to load it from xml defined types first
    type = xmlparser::XMLProfileManager::CreateDynamicPubSubType(name);
    if (type == nullptr)
    {
        if (typesLibs.find(name) != typesLibs.end()) // Has its own library?
        {
            typef_t func = typesLibs[name];
            if (func)
            {
                type = func(name.c_str());
            }
            else
            {
                for (typef_t func : defaultTypesLibs) // A default library defines it?
                {
                    type = func(name.c_str());
                    if (type != nullptr)
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        // Show warning if defined in several ways.
        if (typesLibs.find(name) != typesLibs.end()) // Has its own library?
        {
            typef_t func = typesLibs[name];
            if (func)
            {
                LOG_WARN("Type " << name << " already defined by xml, but found in library.");
            }
            else
            {
                for (typef_t func : defaultTypesLibs) // A default library defines it?
                {
                    if (func(name.c_str()))
                    {
                        LOG_WARN("Type " << name << " already defined by xml, but found in default library.");
                        break;
                    }
                }
            }
        }
    }

    if (type == nullptr)
    {
        // Finally use the Generic
        type = new GenericPubSubType();
        type->setName(name.c_str());
    }
    return type;
}

void ISManager::createSubscriber(Participant* participant, const std::string &name)
{
    RTPSSubscriber* listener = new RTPSSubscriber(getEndPointName(participant->getAttributes().rtps.getName(), name));
    listener->setParticipant(participant);
    if(!listener->hasParticipant())
    {
        LOG_ERROR("Error creating subscriber");
        return;
    }

    // Check type
    SubscriberAttributes subscriber_att;
    if ( xmlparser::XMLP_ret::XML_OK ==
         xmlparser::XMLProfileManager::fillSubscriberAttributes(name, subscriber_att))
    {
        std::string topic_type = subscriber_att.topic.getTopicDataType();
        TopicDataType *type = nullptr;
        if (!Domain::getRegisteredType(participant, topic_type.c_str(), &type))
        {
            // Type not registered yet.
            type = getTopicDataType(topic_type);
            if (type != nullptr)
            {
                Domain::registerType(participant, type);
                std::pair<std::string, std::string> idx =
                        std::make_pair(std::string(participant->getAttributes().rtps.getName()), topic_type);
                data_types[idx] = type;
            }
            else
            {
                LOG_WARN("Cannot determine the TopicDataType of " << name << ".");
            }
        }
    }
    else
    {
        LOG_WARN("Cannot get subscriber attributes: The subscriber " << name << " creation probably will fail.");
    }

    // Create Subscriber
    eprosima::fastrtps::Subscriber *subscriber =
            Domain::createSubscriber(participant, name, (SubscriberListener*)listener);

    listener->setRTPSSubscriber(subscriber);

    if(!listener->hasRTPSSubscriber())
    {
        LOG_ERROR("Error creating subscriber");
        return;
    }

    //Associate types
    const std::string &typeName = listener->getRTPSSubscriber()->getAttributes().topic.topicDataType;
    const std::string &topic_name = listener->getRTPSSubscriber()->getAttributes().topic.topicName;
    std::pair<std::string, std::string> idx =
        std::make_pair(std::string(participant->getAttributes().rtps.getName()), typeName);
    listener->input_type = data_types[idx];
    listener->input_type->setName(typeName.c_str());

    addSubscriber(listener);
    LOG_INFO("Added subscriber " << listener->getName() << "[" << topic_name << ":"
        << participant->getAttributes().rtps.builtin.domainId << "]");
}

void ISManager::createPublisher(Participant* participant, const std::string &name)
{
    RTPSPublisher* publisher = new RTPSPublisher(getEndPointName(participant->getAttributes().rtps.getName(), name));

    // Create RTPSParticipant
    publisher->setParticipant(participant);
    if(!publisher->hasParticipant())
    {
        delete publisher;
        LOG_ERROR("Error creating publisher");
        return;
    }

    // Check type
    PublisherAttributes publisher_att;
    if ( xmlparser::XMLP_ret::XML_OK ==
         xmlparser::XMLProfileManager::fillPublisherAttributes(name, publisher_att))
    {
        std::string topic_type = publisher_att.topic.getTopicDataType();
        TopicDataType *type = nullptr;
        if (!Domain::getRegisteredType(participant, topic_type.c_str(), &type))
        {
            // Type not registered yet.
            type = getTopicDataType(topic_type);
            if (type != nullptr)
            {
                Domain::registerType(participant, type);
                std::pair<std::string, std::string> idx =
                        std::make_pair(std::string(participant->getAttributes().rtps.getName()), topic_type);
                data_types[idx] = type;
            }
            else
            {
                LOG_WARN("Cannot determine the TopicDataType of " << name << ".");
            }
        }
    }
    else
    {
        LOG_WARN("Cannot get publisher attributes: The publisher " << name << " creation probably will fail.");
    }

    //Create publisher
    publisher->setRTPSPublisher(Domain::createPublisher(publisher->getParticipant(), name,
                                (PublisherListener*)publisher));

    //Associate types
    const std::string &typeName = publisher->getRTPSPublisher()->getAttributes().topic.topicDataType;
    const std::string &topic_name = publisher->getRTPSPublisher()->getAttributes().topic.topicName;
    std::pair<std::string, std::string> idx =
        std::make_pair(std::string(participant->getAttributes().rtps.getName()), typeName);
    publisher->output_type = data_types[idx];
    publisher->output_type->setName(typeName.c_str());

    //Create publisher
    //publisher->setRTPSPublisher(Domain::createPublisher(publisher->getParticipant(), name,
    //                            (PublisherListener*)publisher));
    if(!publisher->hasRTPSPublisher())
    {
        delete publisher;
        LOG_ERROR("Error creating publisher");
        return;
    }

    addPublisher(publisher);
    LOG_INFO("Added publisher " << publisher->getName()<< "[" << topic_name << ":"
        << participant->getAttributes().rtps.builtin.domainId << "]");
}


void ISManager::loadBridge(tinyxml2::XMLElement *bridge_element)
{
    try
    {
        const char* bridge_name = bridge_element->Attribute(s_sName.c_str());
        const char *library = _assignNextElement(bridge_element, s_sLibrary)->GetText();
        tinyxml2::XMLElement *properties = _assignOptionalElement(bridge_element, s_sProperties);
        void *handle = getLibraryHandle(library);
        createBridgef_t create_bridge = (createBridgef_t)eProsimaGetProcAddress(handle, s_sFuncCreateBridge.c_str());
        createSubf_t create_subscriber = (createSubf_t)eProsimaGetProcAddress(handle, s_sFuncCreateSubscriber.c_str());
        createPubf_t create_publisher = (createPubf_t)eProsimaGetProcAddress(handle, s_sFuncCreatePublisher.c_str());

        ISBridge *bridge = nullptr;
        if (create_bridge != nullptr)
        {
            if (properties)
            {
                std::vector<std::pair<std::string, std::string>> configuration;

                parseProperties(properties, configuration);

                bridge = create_bridge(bridge_name, &configuration);
            }
            else
            {
                bridge = create_bridge(bridge_name, nullptr);
            }
        }
        if (bridge == nullptr) // Use the default bridge
        {
            bridge = new RTPSBridge(bridge_name);
        }

        addBridge(bridge);

        tinyxml2::XMLElement *subscribers = bridge_element->FirstChildElement(s_sSubscriber.c_str());
        while (subscribers)
        {
            const char* sub_name = subscribers->Attribute(s_sName.c_str());
            std::vector<std::pair<std::string, std::string>> configuration;
            parseProperties(subscribers, configuration);
            ISSubscriber* sub = create_subscriber(bridge, sub_name, &configuration);
            addSubscriber(bridge->getName(), sub);
            subscribers = subscribers->NextSiblingElement(s_sSubscriber.c_str());
        }

        tinyxml2::XMLElement *publishers = bridge_element->FirstChildElement(s_sPublisher.c_str());
        while (publishers)
        {
            const char* pub_name = publishers->Attribute(s_sName.c_str());
            std::vector<std::pair<std::string, std::string>> configuration;
            parseProperties(publishers, configuration);
            ISPublisher* pub = create_publisher(bridge, pub_name, &configuration);
            addPublisher(bridge->getName(), pub);
            publishers = publishers->NextSiblingElement(s_sPublisher.c_str());
        }
    }
    catch (int e_code)
    {
        LOG_ERROR("Error ocurred while loading bridge " << e_code);
    }
}

Participant* ISManager::getParticipant(const std::string &name)
{
    Participant* participant = nullptr;
    // Ensure isn't a bridge
    if (bridges.find(name) == bridges.end())
    {
        if (rtps_participants.find(name) != rtps_participants.end())
        {
            participant = rtps_participants[name];
        }
        else
        {
            participant = Domain::createParticipant(name, nullptr, true); // Use profile name as participant name
            rtps_participants[name] = participant;

            // Register its types
            for (auto &pair : to_register_types)
            {
                if (pair.first == name)
                {
                    TopicDataType* type = data_types[pair];
                    Domain::registerType(participant, type);
                }
            }
        }
    }
    return participant;
}

void ISManager::loadConnector(tinyxml2::XMLElement *connector_element)
{
    try
    {
        const char* connector_name = connector_element->Attribute(s_sName.c_str());
        tinyxml2::XMLElement *sub_el = _assignNextElement(connector_element, s_sSubscriber);
        tinyxml2::XMLElement *pub_el = _assignNextElement(connector_element, s_sPublisher);
        tinyxml2::XMLElement *trans_el = _assignOptionalElement(connector_element, s_sTransformation);

        const char* sub_part   = sub_el->Attribute(s_sParticipantName.c_str());
        const char* sub_bridge = sub_el->Attribute(s_sBridgeName.c_str());
        const char* sub_name   = sub_el->Attribute(s_sSubscriberName.c_str());
        const char* pub_part   = pub_el->Attribute(s_sParticipantName.c_str());
        const char* pub_bridge = pub_el->Attribute(s_sBridgeName.c_str());
        const char* pub_name   = pub_el->Attribute(s_sPublisherName.c_str());

        if (sub_part == nullptr && sub_bridge == nullptr)
        {
            LOG_ERROR("Neither subscriber's participant or bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (sub_part != nullptr && sub_bridge != nullptr)
        {
            LOG_ERROR("Both subscriber's participant and bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (pub_part == nullptr && pub_bridge == nullptr)
        {
            LOG_ERROR("Neither publisher's participant or bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (pub_part != nullptr && pub_bridge != nullptr)
        {
            LOG_ERROR("Both publisher's participant and bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        std::string subName;
        std::string pubName;
        if (sub_part != nullptr)
        {
            Participant* participant_subscriber = getParticipant(sub_part);

            if (participant_subscriber != nullptr)
            {
                createSubscriber(participant_subscriber, sub_name);
                subName = getEndPointName(sub_part, sub_name);
            }
        }
        else
        {
            // Bridge's load created it
            subName = getEndPointName(sub_bridge, sub_name);
        }

        if (pub_part != nullptr)
        {
            Participant* participant_publisher = getParticipant(pub_part);

            if (participant_publisher != nullptr)
            {
                createPublisher(participant_publisher, pub_name);
                pubName = getEndPointName(pub_part, pub_name);
            }
        }
        else
        {
            // Bridge's load created it
            pubName = getEndPointName(pub_bridge, pub_name);
        }

        auto its = subscribers.find(subName);
        if (its == subscribers.end())
        {
            LOG_ERROR("Subscriber " << sub_name << " of participant " << sub_part << " cannot be found.");
            throw 0;
        }

        auto itp = publishers.find(pubName);
        if (itp == publishers.end())
        {
            LOG_ERROR("Publisher " << pub_name << " of participant " << pub_part << " cannot be found.");
            throw 0;
        }

        ISSubscriber* sub = its->second;
        ISPublisher* pub = itp->second;

        std::string function_name;
        userf_t function = nullptr;
        userdynf_t dynFunction = nullptr;

        if (trans_el)
        {
            const char* f_file = trans_el->Attribute(s_sFile.c_str());
            const char* f_name = trans_el->Attribute(s_sFunction.c_str());
            function_name = std::string(f_file) + "@" + f_name;
            void* handle = getLibraryHandle(f_file);

            if (pub->output_type == nullptr || dynamic_cast<DynamicPubSubType*>(pub->output_type) == nullptr)
            {
                function = (userf_t)eProsimaGetProcAddress(handle, f_name);
            }
            else
            {
                dynFunction = (userdynf_t)eProsimaGetProcAddress(handle, f_name);
            }
        }

        // Any participant is a bridge?
        ISBridge* bridge;

        auto itsb = sub_bridge != nullptr ? bridges.find(sub_bridge) : bridges.end();
        auto itpb = pub_bridge != nullptr ? bridges.find(pub_bridge) : bridges.end();

        if (itsb == bridges.end() && itpb == bridges.end())
        {
            // Create the RTPS bridge
            bridge = new RTPSBridge(connector_name);
            addBridge(bridge);
        }
        else if (itsb != bridges.end() && itpb != bridges.end())
        {
            // No RTPS endpoint?
            LOG_ERROR("Connector " << connector_name << " without RTPS endpoint!");
            throw 0; // Not allowed.
        }
        else if(itsb != bridges.end())
        {
            // Subscriber has the custom bridge
            bridge = itsb->second;
        }
        else // itpb != bridges.end()
        {
            // Publisher has the custom bridge
            bridge = itpb->second;
        }

        bridge->addSubscriber(sub);
        bridge->addPublisher(sub->getName(), function_name, pub);
        if (dynFunction == nullptr)
        {
            bridge->addFunction(sub->getName(), function_name, function);
        }
        else
        {
            bridge->addFunction(sub->getName(), function_name, dynFunction);
        }

        LOG_INFO("Set connector between " << sub->getName() << " and " << pub->getName());
    }
    catch (int e_code)
    {
        LOG_ERROR("Error ocurred while loading connector " << e_code);
    }
}

void ISManager::parseProperties(tinyxml2::XMLElement *parent_element,
                                std::vector<std::pair<std::string, std::string>> &props)
{
    tinyxml2::XMLElement *props_element = parent_element->FirstChildElement(s_sProperty.c_str());
    while (props_element)
    {
        try
        {
            std::pair<std::string, std::string> newPair;
            const char *type = _assignNextElement(props_element, s_sName.c_str())->GetText();
            const char *value = _assignNextElement(props_element, s_sValue.c_str())->GetText();
            newPair.first = type;
            newPair.second = value;
            props.emplace_back(newPair);
        }
        catch (...) {}
        props_element = props_element->NextSiblingElement(s_sProperty.c_str());
    }
}

void ISManager::onTerminate()
{
    for (const auto &p : bridges)
    {
        ISBridge *b = p.second;
        if (b)
        {
            b->onTerminate();
        }
    }
}

bool ISManager::isActive()
{
    return active;
}

void* ISManager::getLibraryHandle(const std::string &libpath)
{
    auto it = handles.find(libpath);

    if (it == handles.end())
    {
        void *handle = eProsimaLoadLibrary(libpath.c_str());
        if (handle)
        {
            handles[libpath] = handle;
        }
        else
        {
            LOG_WARN("Cannot load library " << libpath);
        }
    }

    return handles[libpath];
}

ISManager::~ISManager()
{
    onTerminate();

    for (auto it = rtps_participants.rbegin(); it != rtps_participants.rend(); ++it)
    {
        for (auto dataIt = data_types.begin(); dataIt != data_types.end(); ++dataIt)
        {
            if (it->first == dataIt->first.first)
            {
                Domain::unregisterType(it->second, dataIt->first.second.c_str());
            }
        }
        Domain::removeParticipant(it->second);
    }

    for (const auto &p : bridges)
    {
        ISBridge *b = p.second;
        delete b;
    }
    bridges.clear();
    subscribers.clear();
    publishers.clear();

    for (auto it = data_types.begin(); it != data_types.end(); ++it)
    {
        delete it->second;
    }
    data_types.clear();

    for (const auto &p : handles)
    {
        void *h = p.second;
        if (h) eProsimaCloseLibrary(h);
    }
    handles.clear();
}
