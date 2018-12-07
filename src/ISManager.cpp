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

#include <tinyxml2.h>
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
static const std::string s_sFuncCreateReader("create_reader");
static const std::string s_sFuncCreateWriter("create_writer");
static const std::string s_sTransformation("transformation");
static const std::string s_sParticipantProfile("participant_profile");
static const std::string s_sBridgeName("bridge_name");
static const std::string s_sReaderName("reader_name");
static const std::string s_sWriterName("writer_name");
static const std::string s_sReader("reader");
static const std::string s_sWriter("writer");
static const std::string s_sSubscriberProfile("subscriber_profile");
static const std::string s_sPublisherProfile("publisher_profile");
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
static const std::string s_sISTypes("is_types");
static const std::string s_sTypesLibrary("types_library");


ISManager::ISManager(const std::string &xml_file_path)
    : active(false)
{
    //Log::SetVerbosity(Log::Warning);
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xml_file_path.c_str()) == tinyxml2::XMLError::XML_SUCCESS)
    {
        tinyxml2::XMLElement *bridge_element = doc.FirstChildElement(s_sIS.c_str());
        if (!bridge_element)
        {
            LOG("Invalid config file");
            return;
        }

        for (auto child = doc.FirstChildElement(s_sIS.c_str());
            child != nullptr; child = child->NextSiblingElement(s_sIS.c_str()))
        {

            tinyxml2::XMLElement *topic_types = child->FirstChildElement(s_sISTypes.c_str());
            if (topic_types)
            {
                loadISTypes(topic_types);
            }

            tinyxml2::XMLElement *profiles = child->FirstChildElement(s_sProfiles.c_str());
            if (profiles)
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
            while (bridge)
            {
                loadBridge(bridge);
                bridge = bridge->NextSiblingElement(s_sBridge.c_str());
            }

            tinyxml2::XMLElement *connector = child->FirstChildElement(s_sConnector.c_str());
            while (connector)
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
    else
    {
        LOG("Config file not found.");
    }
}

void ISManager::addBridge(ISBridge* b)
{
    //bridge.emplace_back(b);
    bridges[b->getName()] = b;
    active = true;
}

void ISManager::addReader(ISReader* s)
{
    readers[s->getName()] = s;
}

void ISManager::addWriter(ISWriter* p)
{
    writers[p->getName()] = p;
}

void ISManager::addReader(const std::string &part_name, ISReader* s)
{
    readers[getEndPointName(part_name, s->getName())] = s;
}

void ISManager::addWriter(const std::string &part_name, ISWriter* p)
{
    writers[getEndPointName(part_name, p->getName())] = p;
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

void ISManager::loadISTypes(tinyxml2::XMLElement *is_types_element)
{
    try
    {
        tinyxml2::XMLElement *types = is_types_element->FirstChildElement(s_sTypes.c_str());
        if (types)
        {
            loadDynamicTypes(is_types_element);
        }

        tinyxml2::XMLElement *type = is_types_element->FirstChildElement(s_sType.c_str());
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

            type = type->NextSiblingElement(s_sType.c_str());
        }

        tinyxml2::XMLElement *typesLib = is_types_element->FirstChildElement(s_sTypesLibrary.c_str());
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

void ISManager::createReader(Participant* participant, const std::string &participant_profile, const std::string &name)
{
    RTPSSubscriber* listener = new RTPSSubscriber(getEndPointName(participant_profile, name));
    listener->setParticipant(participant);
    if(!listener->hasParticipant())
    {
        LOG_ERROR("Error creating RTPS subscriber");
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
                std::pair<std::string, std::string> idx = std::make_pair(participant_profile, topic_type);
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
        LOG_WARN("Cannot get RTPS subscriber attributes: The subscriber " << name << " creation probably will fail.");
    }

    // Create Subscriber
    eprosima::fastrtps::Subscriber *subscriber =
            Domain::createSubscriber(participant, name, (SubscriberListener*)listener);

    listener->setRTPSSubscriber(subscriber);

    if(!listener->hasRTPSSubscriber())
    {
        LOG_ERROR("Error creating RTPS subscriber");
        return;
    }

    //Associate types
    const std::string &typeName = listener->getRTPSSubscriber()->getAttributes().topic.topicDataType;
    const std::string &topic_name = listener->getRTPSSubscriber()->getAttributes().topic.topicName;
    std::pair<std::string, std::string> idx = std::make_pair(participant_profile, typeName);
    listener->input_type = data_types[idx];
    listener->input_type->setName(typeName.c_str());

    addReader(listener);
    LOG_INFO("Added RTPS subscriber " << listener->getName() << "[" << topic_name << ":"
        << participant->getAttributes().rtps.builtin.domainId << "]");
}

void ISManager::createWriter(Participant* participant, const std::string &participant_profile, const std::string &name)
{
    RTPSPublisher* publisher = new RTPSPublisher(getEndPointName(participant_profile, name));

    // Create RTPSParticipant
    publisher->setParticipant(participant);
    if(!publisher->hasParticipant())
    {
        delete publisher;
        LOG_ERROR("Error creating RTPS publisher");
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
                std::pair<std::string, std::string> idx = std::make_pair(participant_profile, topic_type);
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
        LOG_WARN("Cannot get RTPS publisher attributes: The publisher " << name << " creation probably will fail.");
    }

    //Create publisher
    publisher->setRTPSPublisher(Domain::createPublisher(publisher->getParticipant(), name,
                                (PublisherListener*)publisher));

    //Associate types
    const std::string &typeName = publisher->getRTPSPublisher()->getAttributes().topic.topicDataType;
    const std::string &topic_name = publisher->getRTPSPublisher()->getAttributes().topic.topicName;
    std::pair<std::string, std::string> idx = std::make_pair(participant_profile, typeName);
    publisher->output_type = data_types[idx];
    publisher->output_type->setName(typeName.c_str());

    //Create publisher
    //publisher->setRTPSPublisher(Domain::createWriter(publisher->getParticipant(), name,
    //                            (PublisherListener*)publisher));
    if(!publisher->hasRTPSPublisher())
    {
        delete publisher;
        LOG_ERROR("Error creating RTPS publisher");
        return;
    }

    addWriter(publisher);
    LOG_INFO("Added RTPS publisher " << publisher->getName()<< "[" << topic_name << ":"
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
        createSubf_t create_reader = (createSubf_t)eProsimaGetProcAddress(handle, s_sFuncCreateReader.c_str());
        createPubf_t create_writer = (createPubf_t)eProsimaGetProcAddress(handle, s_sFuncCreateWriter.c_str());

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

        tinyxml2::XMLElement *readers = bridge_element->FirstChildElement(s_sReader.c_str());
        while (readers)
        {
            const char* sub_name = readers->Attribute(s_sName.c_str());
            std::vector<std::pair<std::string, std::string>> configuration;
            parseProperties(readers, configuration);
            ISReader* sub = create_reader(bridge, sub_name, &configuration);
            addReader(bridge->getName(), sub);
            readers = readers->NextSiblingElement(s_sReader.c_str());
        }

        tinyxml2::XMLElement *writers = bridge_element->FirstChildElement(s_sWriter.c_str());
        while (writers)
        {
            const char* pub_name = writers->Attribute(s_sName.c_str());
            std::vector<std::pair<std::string, std::string>> configuration;
            parseProperties(writers, configuration);
            ISWriter* pub = create_writer(bridge, pub_name, &configuration);
            addWriter(bridge->getName(), pub);
            writers = writers->NextSiblingElement(s_sWriter.c_str());
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
            participant = Domain::createParticipant(name, nullptr); // Use profile name as participant name
            rtps_participants[name] = participant;
        }
    }
    return participant;
}

void ISManager::loadConnector(tinyxml2::XMLElement *connector_element)
{
    try
    {
        const char* connector_name = connector_element->Attribute(s_sName.c_str());
        tinyxml2::XMLElement *sub_el = _assignNextElement(connector_element, s_sReader);
        tinyxml2::XMLElement *pub_el = _assignNextElement(connector_element, s_sWriter);
        tinyxml2::XMLElement *trans_el = _assignOptionalElement(connector_element, s_sTransformation);

        const char* sub_part   = sub_el->Attribute(s_sParticipantProfile.c_str());
        const char* sub_bridge = sub_el->Attribute(s_sBridgeName.c_str());
        const char* sub_name = sub_el->Attribute(s_sReaderName.c_str());
        const char* sub_profile_name = sub_el->Attribute(s_sSubscriberProfile.c_str());
        const char* pub_part   = pub_el->Attribute(s_sParticipantProfile.c_str());
        const char* pub_bridge = pub_el->Attribute(s_sBridgeName.c_str());
        const char* pub_name = pub_el->Attribute(s_sWriterName.c_str());
        const char* pub_profile_name = pub_el->Attribute(s_sPublisherProfile.c_str());

        if (sub_part == nullptr && sub_bridge == nullptr)
        {
            LOG_ERROR("Neither reader's participant or bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (sub_part != nullptr && sub_bridge != nullptr)
        {
            LOG_ERROR("Both reader's participant and bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (pub_part == nullptr && pub_bridge == nullptr)
        {
            LOG_ERROR("Neither writer's participant or bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (pub_part != nullptr && pub_bridge != nullptr)
        {
            LOG_ERROR("Both writer's participant and bridge are defined in connector " << connector_name << ".");
            throw 0;
        }

        if (pub_part != nullptr && pub_profile_name == nullptr)
        {
            LOG_ERROR("A RTPS Publisher participant needs a " << s_sPublisherProfile << " in connector "
                << connector_name << ".");
            throw 0;
        }

        if (sub_part != nullptr && sub_profile_name == nullptr)
        {
            LOG_ERROR("A RTPS Subscriber participant needs a " << s_sSubscriberProfile << " in connector "
                << connector_name << ".");
            throw 0;
        }

        if (pub_bridge != nullptr && pub_name == nullptr)
        {
            LOG_ERROR("A Writer bridge needs a " << s_sWriterName << " in connector " << connector_name << ".");
            throw 0;
        }

        if (sub_bridge != nullptr && sub_name == nullptr)
        {
            LOG_ERROR("A Reader bridge needs a " << s_sReaderName << " in connector " << connector_name << ".");
            throw 0;
        }

        std::string subName;
        std::string pubName;
        if (sub_part != nullptr)
        {
            Participant* participant_subscriber = getParticipant(sub_part);
            if (participant_subscriber != nullptr)
            {
                createReader(participant_subscriber, sub_part, sub_profile_name);
                subName = getEndPointName(sub_part, sub_profile_name);
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
                createWriter(participant_publisher, pub_part, pub_profile_name);
                pubName = getEndPointName(pub_part, pub_profile_name);
            }
        }
        else
        {
            // Bridge's load created it
            pubName = getEndPointName(pub_bridge, pub_name);
        }

        auto its = readers.find(subName);
        if (its == readers.end())
        {
            LOG_ERROR("Reader " << sub_name << " of participant " << sub_part << " cannot be found.");
            throw 0;
        }

        auto itp = writers.find(pubName);
        if (itp == writers.end())
        {
            LOG_ERROR("Writer " << pub_name << " of participant " << pub_part << " cannot be found.");
            throw 0;
        }

        ISReader* sub = its->second;
        ISWriter* pub = itp->second;

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
            // Reader has the custom bridge
            bridge = itsb->second;
        }
        else // itpb != bridges.end()
        {
            // Writer has the custom bridge
            bridge = itpb->second;
        }

        bridge->addReader(sub);
        bridge->addWriter(sub->getName(), function_name, pub);
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
    readers.clear();
    writers.clear();

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
