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
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include "ShapePubSubTypes.h"

// String literals
static const std::string s_sIS("is");
static const std::string s_sParticipant("participant");
static const std::string s_sBridge("bridge");
static const std::string s_sConnector("connector");
static const std::string s_sName("name");
static const std::string s_sAttributes("attributes");
static const std::string s_sDomain("domain");
static const std::string s_sSubscriber("subscriber");
static const std::string s_sPublisher("publisher");
static const std::string s_sTopic("topic");
static const std::string s_sType("type");
static const std::string s_sPartition("partition");
static const std::string s_sLibrary("library");
static const std::string s_sProperties("properties");
static const std::string s_sProperty("property");
static const std::string s_sFuncCreateBridge("create_bridge");
static const std::string s_sFuncCreateSubscriber("create_subscriber");
static const std::string s_sFuncCreatePublisher("create_publisher");
static const std::string s_sTransformation("transformation");
static const std::string s_sParticipantName("participant_name");
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

ISManager::ISManager(const std::string &xml_file_path)
    : active(false)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xml_file_path.c_str());

    tinyxml2::XMLElement *bridge_element = doc.FirstChildElement(s_sIS.c_str());
    if (!bridge_element)
    {
        LOG("Invalid config file");
        return;
    }

    for (auto child = doc.FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
    {
        tinyxml2::XMLElement *participant = child->FirstChildElement(s_sParticipant.c_str());
        while(participant)
        {
            loadParticipant(participant);
            participant = participant->NextSiblingElement(s_sParticipant.c_str());
        }

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

void ISManager::loadParticipant(tinyxml2::XMLElement *participant_element)
{
    try
    {
        const char* part_name = participant_element->Attribute(s_sName.c_str());
        if (!part_name)
        {
           LOG("Found participant without name.");
           throw 0;
        }

        tinyxml2::XMLElement *attribs = participant_element->FirstChildElement(s_sAttributes.c_str());
        if (!attribs)
        {
            LOG("No attributes found for participant " << part_name);
            throw 0;
        }

        int output_domain = 0;
        tinyxml2::XMLElement *current_element = _assignNextElement(attribs, s_sDomain);
        if(current_element->QueryIntText(&output_domain))
        {
            LOG("Cannot parse domain for participant ");
            throw 0;
        }

        // Participant configuration
        ParticipantAttributes part_params;

        // Is TCP?
        current_element = _assignOptionalElement(attribs, s_sProtocol);
        //current_element = attribs->FirstChildElement(s_sProtocol.c_str());
        const char* protocol = (current_element == nullptr) ? nullptr : current_element->GetText();
        if (protocol != nullptr && strncmp(protocol, "tcp", 3) == 0)
        {
            const char* address = _assignNextElement(attribs, s_sRemoteAddress)->GetText();
            int remotePort, localPort;
            current_element = _assignNextElement(attribs, s_sRemotePort);
            if(current_element->QueryIntText(&remotePort))
            {
                LOG("Cannot parse remote port for TCP participant ");
                throw 0;
            }
            current_element = _assignNextElement(attribs, s_sListeningPort);
            if(current_element->QueryIntText(&localPort))
            {
                LOG("Cannot parse listening port for TCP participant ");
                throw 0;
            }

            std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
            descriptor->listening_ports.emplace_back(localPort);
            descriptor->sendBufferSize = 0;
            descriptor->receiveBufferSize = 0;
            descriptor->set_WAN_address(address);

            Locator_t initial_peer_locator;
            initial_peer_locator.kind = LOCATOR_KIND_TCPv4;
            initial_peer_locator.set_IP4_address("127.0.0.1");
            initial_peer_locator.set_port(remotePort);
            //initial_peer_locator.set_logical_port(7402);
            // Remote meta channel
            part_params.rtps.builtin.initialPeersList.push_back(initial_peer_locator);

            Locator_t out_locator;
            out_locator.kind = LOCATOR_KIND_TCPv4;
            out_locator.set_IP4_address("127.0.0.1");
            out_locator.set_port(remotePort);
            //out_locator.set_logical_port(7410);
            // Remote data channel
            part_params.rtps.defaultOutLocatorList.push_back(out_locator);

            Locator_t unicast_locator;
            unicast_locator.kind = LOCATOR_KIND_TCPv4;
            unicast_locator.set_IP4_address("127.0.0.1");
            unicast_locator.set_port(localPort);
            //unicast_locator.set_logical_port(7410);
            // Our data channel
            part_params.rtps.defaultUnicastLocatorList.push_back(unicast_locator); 

            Locator_t meta_locator;
            meta_locator.kind = LOCATOR_KIND_TCPv4;
            meta_locator.set_IP4_address("127.0.0.1");
            meta_locator.set_port(localPort);
            //meta_locator.set_logical_port(7402);
            // Our meta channel 
            part_params.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator);  

            part_params.rtps.useBuiltinTransports = false;
            part_params.rtps.userTransports.push_back(descriptor);
        }

        part_params.rtps.builtin.domainId = output_domain;
        part_params.rtps.builtin.leaseDuration = c_TimeInfinite;
        part_params.rtps.setName(part_name);

        Participant* participant = Domain::createParticipant(part_params, &myParticipantListener);
        tinyxml2::XMLElement *subscribers = participant_element->FirstChildElement(s_sSubscriber.c_str());
        while (subscribers)
        {
            loadSubscriber(participant, subscribers);
            subscribers = subscribers->NextSiblingElement(s_sSubscriber.c_str());
        }

        tinyxml2::XMLElement *publishers = participant_element->FirstChildElement(s_sPublisher.c_str());
        while (publishers)
        {
            loadPublisher(participant, publishers);
            publishers = publishers->NextSiblingElement(s_sPublisher.c_str());
        }
    }
    catch (int e_code)
    {
        LOG_ERROR("Error ocurred while loading participant " << e_code);
    }
}

void ISManager::loadSubscriber(Participant* participant, tinyxml2::XMLElement *subscriber_element)
{
    try
    {
        const char* sub_name = subscriber_element->Attribute(s_sName.c_str());
        tinyxml2::XMLElement *attribs = subscriber_element->FirstChildElement(s_sAttributes.c_str());
        if (!attribs)
        {
            LOG("No attributes found for subscriber " << sub_name);
            throw 0;
        }

        tinyxml2::XMLElement *current_element = _assignNextElement(attribs, s_sTopic);
        const char* topic_name = current_element->GetText();
        current_element = _assignNextElement(attribs, s_sType);
        const char* type_name = current_element->GetText();

        current_element = _assignOptionalElement(attribs, s_sPartition);
        const char* partition = (current_element == nullptr) ? nullptr : current_element->GetText();

        // Subscriber configuration
        SubscriberAttributes sub_params;
        sub_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        //sub_params.topic.topicKind = NO_KEY;
        sub_params.topic.topicKind = WITH_KEY;
        sub_params.topic.topicDataType = type_name;
        sub_params.topic.topicName = topic_name;

        // TODO To config?
        sub_params.expectsInlineQos = false;
        sub_params.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        sub_params.topic.historyQos.depth = 5;
        sub_params.qos.m_presentation.hasChanged = true;
        sub_params.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        sub_params.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        sub_params.qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;

        if (partition != nullptr)
        {
            sub_params.qos.m_partition.push_back(partition);
        }

        RTPSSubscriber* listener = new RTPSSubscriber(getEndPointName(participant->getAttributes().rtps.getName(), sub_name));
        listener->setParticipant(participant);
        if(!listener->hasParticipant())
        {
            delete listener;
            throw 0;
        }

        //listener->input_type = new GenericPubSubType();
        listener->input_type = new ShapeTypePubSubType();
        listener->input_type->setName(sub_params.topic.topicDataType.c_str());
        listener->input_type->m_isGetKeyDefined = true;

        // Make sure register this type only once per participant
        std::string typeId = std::string(listener->getParticipant()->getAttributes().rtps.getName()) + listener->input_type->getName();
        auto it = registered_types.find(typeId);
        if (it == registered_types.end() || !(*it).second)
        {
            Domain::registerType(listener->getParticipant(),(TopicDataType*) listener->input_type);
            registered_types[typeId] = true;
        }

        // Create Subscriber
        listener->setRTPSSubscriber(Domain::createSubscriber(listener->getParticipant(), sub_params,
                                                        (SubscriberListener*)listener));
        if(!listener->hasRTPSSubscriber())
        {
            LOG_ERROR("Error ocurred while loading subscriber");
            delete listener;
            return;
        }

        addSubscriber(listener);
        LOG_INFO("Added subscriber " << listener->getName() << "[" << topic_name << ":"
            << participant->getAttributes().rtps.builtin.domainId << "]");
    }
    catch (int e_code)
    {
        LOG_ERROR("Error ocurred while loading subscriber " << e_code);
    }
}

void ISManager::loadPublisher(Participant* participant, tinyxml2::XMLElement *publisher_element)
{
    try
    {
        const char* pub_name = publisher_element->Attribute(s_sName.c_str());
        tinyxml2::XMLElement *attribs = publisher_element->FirstChildElement(s_sAttributes.c_str());
        if (!attribs)
        {
            LOG("No attributes found for publisher " << pub_name);
            throw 0;
        }

        tinyxml2::XMLElement *current_element = _assignNextElement(attribs, s_sTopic);
        const char* topic_name = current_element->GetText();
        current_element = _assignNextElement(attribs, s_sType);
        const char* type_name = current_element->GetText();

        current_element = _assignOptionalElement(attribs, s_sPartition);
        const char* partition = (current_element == nullptr) ? nullptr : current_element->GetText();

        // Publisher configuration
        PublisherAttributes pub_params;
        pub_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        //pub_params.topic.topicKind = NO_KEY;
        pub_params.topic.topicKind = WITH_KEY;
        pub_params.topic.topicDataType = type_name;
        pub_params.topic.topicName = topic_name;

        // TODO to config?
        pub_params.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
        pub_params.topic.historyQos.depth = 5;
        pub_params.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        pub_params.qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
        pub_params.qos.m_liveliness.lease_duration = c_TimeInfinite;
        pub_params.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        pub_params.qos.m_ownership.kind = SHARED_OWNERSHIP_QOS;

        if (partition != nullptr)
        {
            pub_params.qos.m_partition.push_back(partition);
        }

        RTPSPublisher* publisher = new RTPSPublisher(getEndPointName(participant->getAttributes().rtps.getName(), pub_name));

        // Create RTPSParticipant
        publisher->setParticipant(participant);
        if(!publisher->hasParticipant())
        {
            delete publisher;
            throw 0;
        }

        //Register types
        //publisher->output_type = new GenericPubSubType();
        publisher->output_type = new ShapeTypePubSubType();        
        publisher->output_type->setName(pub_params.topic.topicDataType.c_str());
        publisher->output_type->m_isGetKeyDefined = true;

        // Make sure register this type only once per participant
        std::string typeId = std::string(publisher->getParticipant()->getAttributes().rtps.getName()) + publisher->output_type->getName();
        auto it = registered_types.find(typeId);
        if (it == registered_types.end() || !(*it).second)
        {
            Domain::registerType(publisher->getParticipant(), (TopicDataType*)publisher->output_type);
            registered_types[typeId] = true;
        }

        //Create publisher
        publisher->setRTPSPublisher(Domain::createPublisher(publisher->getParticipant(), pub_params,
                                   (PublisherListener*)publisher));
        if(!publisher->hasRTPSPublisher())
        {
            delete publisher;
            throw 0;
        }

        addPublisher(publisher);
        LOG_INFO("Added publisher " << publisher->getName()<< "[" << topic_name << ":"
            << participant->getAttributes().rtps.builtin.domainId << "]");
    }
    catch (int e_code)
    {
        LOG_ERROR("Error ocurred while loading publisher " << e_code);
    }
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

void ISManager::loadConnector(tinyxml2::XMLElement *connector_element)
{
    try
    {
        const char* connector_name = connector_element->Attribute(s_sName.c_str());
        tinyxml2::XMLElement *sub_el = _assignNextElement(connector_element, s_sSubscriber);
        tinyxml2::XMLElement *pub_el = _assignNextElement(connector_element, s_sPublisher);
        tinyxml2::XMLElement *trans_el = _assignOptionalElement(connector_element, s_sTransformation);

        const char* sub_part = sub_el->Attribute(s_sParticipantName.c_str());
        const char* sub_name = sub_el->Attribute(s_sSubscriberName.c_str());
        const char* pub_part = pub_el->Attribute(s_sParticipantName.c_str());
        const char* pub_name = pub_el->Attribute(s_sPublisherName.c_str());

        std::string subName = getEndPointName(sub_part, sub_name);
        std::string pubName = getEndPointName(pub_part, pub_name);

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
        if (trans_el)
        {
            const char* f_file = trans_el->Attribute(s_sFile.c_str());
            const char* f_name = trans_el->Attribute(s_sFunction.c_str());
            function_name = std::string(f_file) + "@" + f_name;
            void* handle = getLibraryHandle(f_file);
            function = (userf_t)eProsimaGetProcAddress(handle, f_name);
        }

        // Any participant is a bridge?
        ISBridge* bridge;
        auto itsb = bridges.find(sub_part);
        auto itpb = bridges.find(pub_part);
        if (itsb == bridges.end() && itpb == bridges.end())
        {
            // Create the RTPS bridge
            bridge = (ISBridge*)new RTPSBridge(connector_name);
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
        bridge->addFunction(sub->getName(), function_name, function);

        LOG_INFO("Set bridge between " << sub->getName() << " and " << pub->getName());
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

    for (const auto &p : handles)
    {
        void *h = p.second;
        if(h) eProsimaCloseLibrary(h);
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
    for (const auto &p : bridges)
    {
        ISBridge *b = p.second;
        delete b;
    }
    bridges.clear();
    subscribers.clear();
    publishers.clear();
    handles.clear();
}
