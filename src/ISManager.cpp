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
#include "ISBridgeRTPS.h"
#include <fastrtps/Domain.h>

ISManager::ISManager(const std::string &xml_file_path) : active(false)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xml_file_path.c_str());

    tinyxml2::XMLElement *bridge_element = doc.FirstChildElement("is");

    if (!bridge_element)
    {
        std::cout << "Invalid config file" << std::endl;
        return;
    }

    for (auto child = doc.FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
    {
        tinyxml2::XMLElement *participant = child->FirstChildElement("participant");
        while(participant)
        {
            loadParticipant(participant);
            participant = participant->NextSiblingElement("participant");
        }

        tinyxml2::XMLElement *bridge = child->FirstChildElement("bridge");
        while(bridge)
        {
            loadBridge(bridge);
            bridge = bridge->NextSiblingElement("bridge");
        }

        tinyxml2::XMLElement *connector = child->FirstChildElement("connector");
        while(connector)
        {
            loadConnector(connector);
            connector = connector->NextSiblingElement("connector");
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
        const char* part_name = participant_element->Attribute("name");
        if (!part_name)
        {
           std::cout << "Found participant without name." << std::endl;
           throw 0;
        }

        tinyxml2::XMLElement *attribs = participant_element->FirstChildElement("attributes");
        if (!attribs)
        {
            std::cout << "No attributes found for participant " << part_name << std::endl;
            throw 0;
        }

        int output_domain = 0;
        tinyxml2::XMLElement *current_element = _assignNextElement(attribs, "domain");
        if(current_element->QueryIntText(&output_domain))
        {
            std::cout << "Cannot parse domain for participant " << part_name << std::endl;
            throw 0;
        }

        // Participant configuration
        ParticipantAttributes part_params;
        part_params.rtps.builtin.domainId = output_domain;
        part_params.rtps.builtin.leaseDuration = c_TimeInfinite;
        part_params.rtps.setName(part_name);

        tinyxml2::XMLElement *subscribers = participant_element->FirstChildElement("subscriber");
        while (subscribers)
        {
            loadSubscriber(part_params, subscribers);
            subscribers = subscribers->NextSiblingElement("subscriber");
        }

        tinyxml2::XMLElement *publishers = participant_element->FirstChildElement("publisher");
        while (publishers)
        {
            loadPublisher(part_params, publishers);
            publishers = publishers->NextSiblingElement("publisher");
        }
    }
    catch (int e_code)
    {
        std::cout << "Error ocurred while loading participant " << e_code << std::endl;
    }
}

void ISManager::loadSubscriber(ParticipantAttributes &part_attrs, tinyxml2::XMLElement *subscriber_element)
{
    try
    {
        const char* sub_name = subscriber_element->Attribute("name");

        tinyxml2::XMLElement *attribs = subscriber_element->FirstChildElement("attributes");
        if (!attribs)
        {
            std::cout << "No attributes found for subscriber " << sub_name << std::endl;
            throw 0;
        }

        tinyxml2::XMLElement *current_element = _assignNextElement(attribs, "topic");
        const char* topic_name = current_element->GetText();
        current_element = _assignNextElement(attribs, "type");
        const char* type_name = current_element->GetText();

        current_element = _assignOptionalElement(attribs, "partition");
        const char* partition = (current_element == nullptr) ? nullptr : current_element->GetText();

        // Subscriber configuration
        SubscriberAttributes sub_params;
        sub_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        sub_params.topic.topicKind = NO_KEY;
        sub_params.topic.topicDataType = type_name;
        sub_params.topic.topicName = topic_name;
        if (partition != nullptr)
        {
            sub_params.qos.m_partition.push_back(partition);
        }

        RTPSListener* listener = new RTPSListener(getEndPointName(part_attrs.rtps.getName(), sub_name));
        listener->setParticipant(Domain::createParticipant(part_attrs));
        if(!listener->hasParticipant())
        {
            delete listener;
            throw 0;
        }

        listener->input_type = new GenericPubSubType();
        listener->input_type->setName(sub_params.topic.topicDataType.c_str());
        Domain::registerType(listener->getParticipant(),(TopicDataType*) listener->input_type);

        // Create Subscriber
        listener->setRTPSSubscriber(Domain::createSubscriber(listener->getParticipant(), sub_params,
                                                        (SubscriberListener*)listener));
        if(!listener->hasRTPSSubscriber())
        {
            delete listener;
            throw 0;
        }

        addSubscriber(listener);
        std::cout << "Added subscriber " << listener->getName()
            << "[" << topic_name << ":" << part_attrs.rtps.builtin.domainId << "]" << std::endl;
    }
    catch (int e_code)
    {
        std::cout << "Error ocurred while loading subscriber " << e_code << std::endl;
    }
}

void ISManager::loadPublisher(ParticipantAttributes &part_attrs, tinyxml2::XMLElement *publisher_element)
{
    try
    {
        const char* pub_name = publisher_element->Attribute("name");

        tinyxml2::XMLElement *attribs = publisher_element->FirstChildElement("attributes");
        if (!attribs)
        {
            std::cout << "No attributes found for publisher " << pub_name << std::endl;
            throw 0;
        }

        tinyxml2::XMLElement *current_element = _assignNextElement(attribs, "topic");
        const char* topic_name = current_element->GetText();
        current_element = _assignNextElement(attribs, "type");
        const char* type_name = current_element->GetText();

        current_element = _assignOptionalElement(attribs, "partition");
        const char* partition = (current_element == nullptr) ? nullptr : current_element->GetText();

        // Publisher configuration
        PublisherAttributes pub_params;
        pub_params.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
        pub_params.topic.topicKind = NO_KEY;
        pub_params.topic.topicDataType = type_name;
        pub_params.topic.topicName = topic_name;
        if (partition != nullptr)
        {
            pub_params.qos.m_partition.push_back(partition);
        }

        RTPSPublisher* publisher = new RTPSPublisher(getEndPointName(part_attrs.rtps.getName(), pub_name));

        // Create RTPSParticipant
        publisher->setParticipant(Domain::createParticipant(part_attrs));
        if(!publisher->hasParticipant())
        {
            delete publisher;
            throw 0;
        }

        //Register types
        publisher->output_type = new GenericPubSubType();
        publisher->output_type->setName(pub_params.topic.topicDataType.c_str());
        Domain::registerType(publisher->getParticipant(), (TopicDataType*)publisher->output_type);

        //Create publisher
        publisher->setRTPSPublisher(Domain::createPublisher(publisher->getParticipant(), pub_params,
                                   (PublisherListener*)publisher));
        if(!publisher->hasRTPSPublisher())
        {
            delete publisher;
            throw 0;
        }

        addPublisher(publisher);
        std::cout << "Added publisher " << publisher->getName()
            << "[" << topic_name << ":" << part_attrs.rtps.builtin.domainId << "]" << std::endl;
    }
    catch (int e_code)
    {
        std::cout << "Error ocurred while loading publisher " << e_code << std::endl;
    }
}


void ISManager::loadBridge(tinyxml2::XMLElement *bridge_element)
{
    try
    {
        const char* bridge_name = bridge_element->Attribute("name");
        const char *library = _assignNextElement(bridge_element, "library")->GetText();
        tinyxml2::XMLElement *properties = _assignOptionalElement(bridge_element, "properties");
        void *handle = getLibraryHandle(library);
        createBridgef_t create_bridge = (createBridgef_t)eProsimaGetProcAddress(handle, "create_bridge");
        createSubf_t create_subscriber = (createSubf_t)eProsimaGetProcAddress(handle, "create_subscriber");
        createPubf_t create_publisher = (createPubf_t)eProsimaGetProcAddress(handle, "create_publisher");

        ISBridge *bridge = nullptr;
        if (properties)
        {
            std::vector<std::pair<std::string, std::string>> configuration;

            parseProperties(bridge_element, configuration);

            bridge = create_bridge(bridge_name, &configuration);
        }
        else
        {
            bridge = create_bridge(bridge_name, nullptr);
        }

        addBridge(bridge);

        tinyxml2::XMLElement *subscribers = bridge_element->FirstChildElement("subscriber");
        while (subscribers)
        {
            const char* sub_name = subscribers->Attribute("name");
            std::vector<std::pair<std::string, std::string>> configuration;
            parseProperties(subscribers, configuration);
            ISSubscriber* sub = create_subscriber(bridge, sub_name, &configuration);
            addSubscriber(bridge->getName(), sub);
            subscribers = subscribers->NextSiblingElement("subscriber");
        }

        tinyxml2::XMLElement *publishers = bridge_element->FirstChildElement("publisher");
        while (publishers)
        {
            const char* pub_name = publishers->Attribute("name");
            std::vector<std::pair<std::string, std::string>> configuration;
            parseProperties(publishers, configuration);
            ISPublisher* pub = create_publisher(bridge, pub_name, &configuration);
            addPublisher(bridge->getName(), pub);
            publishers = publishers->NextSiblingElement("publisher");
        }
    }
    catch (int e_code)
    {
        std::cout << "Error ocurred while loading bridge " << e_code << std::endl;
    }
}

void ISManager::loadConnector(tinyxml2::XMLElement *connector_element)
{
    try
    {
        const char* connector_name = connector_element->Attribute("name");
        tinyxml2::XMLElement *sub_el = _assignNextElement(connector_element, "subscriber");
        tinyxml2::XMLElement *pub_el = _assignNextElement(connector_element, "publisher");
        tinyxml2::XMLElement *trans_el = _assignOptionalElement(connector_element, "transformation");

        const char* sub_part = sub_el->Attribute("participant_name");
        const char* sub_name = sub_el->Attribute("subscriber_name");
        const char* pub_part = pub_el->Attribute("participant_name");
        const char* pub_name = pub_el->Attribute("publisher_name");

        std::string subName = getEndPointName(sub_part, sub_name);
        std::string pubName = getEndPointName(pub_part, pub_name);

        auto its = subscribers.find(subName);
        if (its == subscribers.end())
        {
            std::cout << "Subscriber " << sub_name << " of participant " << sub_part << " cannot be found." << std::endl;
            throw 0;
        }

        auto itp = publishers.find(pubName);
        if (itp == publishers.end())
        {
            std::cout << "Publisher " << pub_name << " of participant " << pub_part << " cannot be found." << std::endl;
            throw 0;
        }

        ISSubscriber* sub = its->second;
        ISPublisher* pub = itp->second;

        std::string function_name;
        userf_t function = nullptr;
        if (trans_el)
        {
            const char* f_file = trans_el->Attribute("file");
            const char* f_name = trans_el->Attribute("function");
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
            bridge = (ISBridge*)new ISBridgeRTPS(connector_name);
            addBridge(bridge);
        }
        else if (itsb != bridges.end() && itpb != bridges.end())
        {
            // No RTPS endpoint?
            std::cout << "Connector " << connector_name << " without RTPS endpoint!" << std::endl;
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

        std::cout << "Set bridge between " << sub->getName() << " and " << pub->getName() << std::endl;
    }
    catch (int e_code)
    {
        std::cout << "Error ocurred while loading connector " << e_code << std::endl;
    }
}

void ISManager::parseProperties(tinyxml2::XMLElement *parent_element,
                                std::vector<std::pair<std::string, std::string>> &props)
{
    tinyxml2::XMLElement *props_element = _assignOptionalElement(parent_element, "properties");
    while (props_element)
    {
        try
        {
            std::pair<std::string, std::string> newPair;
            const char *type = _assignNextElement(props_element, "name")->GetText();
            const char *value = _assignNextElement(props_element, "value")->GetText();
            newPair.first = type;
            newPair.second = value;
            props.emplace_back(newPair);
        }
        catch (...) {}
        props_element = props_element->NextSiblingElement("properties");
    }
}

    /*
    try
    {
        void* handle;

        tinyxml2::XMLElement *lib_element = bridge_element->FirstChildElement("bridge_library");
        tinyxml2::XMLElement *pub_element = bridge_element->FirstChildElement("publisher");
        tinyxml2::XMLElement *sub_element = bridge_element->FirstChildElement("subscriber");

        if (!lib_element && !sub_element && !pub_element)
        {
            std::cout << "You must define at least publisher and subscriber of a \
                bridge_library and publisher or subscriber." << std::endl;
            throw 0;
        }

        ISBridge *bridge = nullptr;

        if (sub_element && pub_element && lib_element)
        {
            std::cout << "Error loading Bridge. Are you trying to make a bidirectional bridge? \
                This is configured as unidirectional " << std::endl;
            throw 0;
        }

        if (sub_element && pub_element && !lib_element) // RTPS Only bridge
        {
            bridge = ISBridgeRTPS::configureBridge(bridge_element);
            if (bridge)
            {
                manager->addBridge(bridge);
            }
            else
            {
                std::cout << "Error loading RTPS bridge configuration. " << std::endl;
                throw 0;
            }
        }
        else if (lib_element)
        {
            const char* file_path = lib_element->GetText();
            handle = eProsimaLoadLibrary(file_path);

            if (handle)
            {
                manager->addHandle(handle);
                loadbridgef_t loadLib = (loadbridgef_t)eProsimaGetProcAddress(handle, "createBridge");
                if (loadLib)
                {
                    tinyxml2::XMLElement *config_element = bridge_element->FirstChildElement("bridge_configuration");
                    if (config_element)
                    {
                        const char* bridge_config = config_element->GetText();

                        if (!bridge_config || bridge_config[0] == '\0')
                        {
                            // Maybe not a string... It is inner xml?
                            tinyxml2::XMLPrinter printer;
                            config_element->Accept(&printer);
                            std::stringstream ss;
                            ss << printer.CStr();

                            std::string temp = ss.str();
                            std::replace(temp.begin(), temp.end(), '\n', ' ');
                            if (!temp.empty())
                            {
                                bridge = loadLib(temp.c_str());
                            }
                            else
                            {
                                std::cout << "INFO: bridge_configuration is empty." << std::endl;
                                bridge = loadLib(nullptr); // Let's try to load without configuration.
                            }
                        }
                        else
                        {
                            bridge = loadLib(bridge_config);
                        }
                    }
                    else
                    {
                        std::cout << "INFO: No bridge_configuration element found." << std::endl;
                        bridge = loadLib(nullptr); // Let's try to load without configuration.
                    }

                    // User bridge failed?
                    if (!bridge)
                    {
                        std::cout << "Error loading bridge configuration " << file_path << std::endl;
                        throw 0;
                    }

                    if (sub_element)
                    {
                        // RTPS -> Other
                        RTPSListener* listener = RTPSListener::configureRTPSSubscriber(sub_element);

                        // Transformation function?
                        const char* function_path;
                        if (bridge_element->FirstChildElement("transformation"))
                        {
                            function_path = bridge_element->FirstChildElement("transformation")->GetText();
                            listener->setTransformation(function_path, "transform");
                        }
                        else
                        {
                            function_path = nullptr;
                        }

                        bridge->setRTPSSubscriber(listener);

                        if (!bridge->getOtherPublisher())
                        {
                            std::cout << "WARNING: Bridge doesn't seem to have a configured Publisher. This"\
                                " may be correct, but you are not using the standard architecture." << std::endl;
                        }
                        else
                        {
                            listener->setPublisher(bridge->getOtherPublisher());
                        }

                    }
                    else if (pub_element)
                    {
                        // Other > RTPS
                        RTPSPublisher* publisher = RTPSPublisher::configureRTPSPublisher(pub_element);
                        bridge->setRTPSPublisher(publisher);
                        if (!bridge->getOtherSubscriber())
                        {
                            std::cout << "WARNING: Bridge doesn't seem to have a configured Subscriber. This"\
                                " may be correct, but you are not using the standard architecture." << std::endl;
                        }
                        else
                        {
                            bridge->getOtherSubscriber()->setPublisher(publisher);
                        }
                    }
                    else
                    {
                        std::cout << "Error no RTPS participant found! " << std::endl;
                        delete bridge;
                        throw 0;
                    }
                    manager->addBridge(bridge);
                }
                else
                {
                    std::cout << "External symbol 'createBridge' in " << file_path << " not found!" << std::endl;
                }
            }
            else
            {
                std::cout << "Bridge library " << file_path << " not found!" << std::endl;
            }
        }
        else
        {
            std::cout << "Bridge library element not found!" << std::endl;
        }
    }
    catch (int e_code){
        std::cout << "Error ocurred while loading bridge library " << e_code << std::endl;
    }
    */
/*
void loadBidirectional(ISManager *manager, tinyxml2::XMLElement *bridge_element)
{
    ISBridge* bridge = nullptr;
    try
    {
        tinyxml2::XMLElement *lib_element = bridge_element->FirstChildElement("bridge_library");
        tinyxml2::XMLElement *rtps_element = bridge_element->FirstChildElement("rtps");

        if(!lib_element)
        {
            std::cout << "No bridge_library element found!" << std::endl;
            throw 0;
        }
        else if (!rtps_element)
        {
            std::cout << "No rtps element found!" << std::endl;
            throw 0;
        }

        const char* file_path = lib_element->GetText();
        void *handle = eProsimaLoadLibrary(file_path);
        if (!handle)
        {
            std::cout << "Bridge library " << file_path << " not found!" << std::endl;
        }
        else
        {
            manager->addHandle(handle);
            loadbridgef_t loadLib = (loadbridgef_t)eProsimaGetProcAddress(handle, "createBridge");

            if (!loadLib)
            {
                std::cout << "External symbol 'createBridge' in " << file_path << " not found!" << std::endl;
            }
            else
            {
                tinyxml2::XMLElement *config_element = bridge_element->FirstChildElement("bridge_configuration");
                if (config_element)
                {
                    const char* bridge_config = config_element->GetText();
                    if (!bridge_config || bridge_config[0] == '\0')
                    {
                        // Maybe not a string... It is inner xml?
                        tinyxml2::XMLPrinter printer;
                        config_element->Accept(&printer);
                        std::stringstream ss;
                        ss << printer.CStr();

                        std::string temp = ss.str();
                        std::replace(temp.begin(), temp.end(), '\n', ' ');
                        if (!temp.empty())
                        {
                            bridge = loadLib(temp.c_str());
                        }
                        else
                        {
                            std::cout << "INFO: bridge_configuration is empty." << std::endl;
                            bridge = loadLib(nullptr); // Let's try to load without configuration.
                        }
                    }
                    else
                    {
                        bridge = loadLib(bridge_config);
                    }
                }
                else
                {
                    std::cout << "INFO: No bridge_configuration element found." << std::endl;
                    bridge = loadLib(nullptr); // Let's try to load without configuration.
                }

                // User bridge failed?
                if (!bridge)
                {
                    std::cout << "Error loading bridge configuration " << file_path << std::endl;
                    throw 0;
                }

                // Load RTPS Participants
                tinyxml2::XMLElement *pub_element = rtps_element->FirstChildElement("publisher");
                tinyxml2::XMLElement *sub_element = rtps_element->FirstChildElement("subscriber");
                if(!sub_element || !pub_element)
                {
                    std::cout << "Error loading rtps participants." << file_path << std::endl;
                    throw 0;
                }

                RTPSListener* listener = RTPSListener::configureRTPSSubscriber(sub_element);
                RTPSPublisher* publisher = RTPSPublisher::configureRTPSPublisher(pub_element);

                if (!listener)
                {
                    std::cout << "Error loading RTPS listener configuration. " << std::endl;
                    throw 0;
                }
                else if (!publisher)
                {
                    std::cout << "Error loading RTPS publisher configuration. " << std::endl;
                    throw 0;
                }

                // Transformation function?
                const char* function_path;
                if (bridge_element->FirstChildElement("transformation"))
                {
                    function_path = bridge_element->FirstChildElement("transformation")->GetText();
                    listener->setTransformation(function_path, "transform");
                }
                else
                {
                    function_path = nullptr;
                }

                bridge->setRTPSPublisher(publisher);
                bridge->setRTPSSubscriber(listener);

                if (!bridge->getOtherPublisher())
                {
                    std::cout << "WARNING: Bridge doesn't seem to have a configured Publisher. This"\
                        " may be correct, but you are not using the standard architecture." << std::endl;
                }
                else
                {
                    listener->setPublisher(bridge->getOtherPublisher());
                }

                if (!bridge->getOtherSubscriber())
                {
                    std::cout << "WARNING: Bridge doesn't seem to have a configured Subscriber. This"\
                        " may be correct, but you are not using the standard architecture." << std::endl;
                }
                else
                {
                    bridge->getOtherSubscriber()->setPublisher(publisher);
                }

                manager->addBridge(bridge);
            }
        }
    }
    catch (int e_code){
        std::cout << "Error ocurred while loading bridge library " << e_code << std::endl;
        if (bridge) delete bridge;
    }
}
*/

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
            std::cout << "Cannot load library " << libpath << std::endl;
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
