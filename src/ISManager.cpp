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
#include "xmlUtils.h"

void loadUnidirectional(ISManager *manager, tinyxml2::XMLElement *bridge_element);
void loadBidirectional(ISManager *manager, tinyxml2::XMLElement *bridge_element);

ISManager::ISManager(std::string xml_file_path) : active(false)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xml_file_path.c_str());

    tinyxml2::XMLElement *bridge_element = doc.FirstChildElement("is");

    if (!bridge_element)
    {
        std::cout << "Invalid config file" << std::endl;
        return;
    }

    for (auto child = bridge_element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
    {
        tinyxml2::XMLElement *current_element = _assignOptionalElement(child, "bridge_type");
        const char* bridge_type = (current_element) ? current_element->GetText() : "unidirectional"; // Old format, only fastrtps support
        if (strncmp(bridge_type, "unidirectional", 14) == 0)
        {
            loadUnidirectional(this, child);
        }
        else if (strncmp(bridge_type, "bidirectional", 13) == 0)
        {
            loadBidirectional(this, child);
        }
    }

    if (bridge.size() > 0)
    {
        active = true;
    }
}

void ISManager::addBridge(ISBridge* b)
{
    bridge.emplace_back(b);
    active = true;
}

void ISManager::addHandle(void* h)
{
    handle.emplace_back(h);
}

void loadUnidirectional(ISManager *manager, tinyxml2::XMLElement *bridge_element)
{
    try
    {
        void* handle;

        tinyxml2::XMLElement *lib_element = bridge_element->FirstChildElement("bridge_library");
        tinyxml2::XMLElement *pub_element = bridge_element->FirstChildElement("publisher");
        tinyxml2::XMLElement *sub_element = bridge_element->FirstChildElement("subscriber");

        if (!lib_element && !sub_element && !pub_element)
        {
            std::cout << "You must define at least publisher ans subscriber of a bridge_library and publisher or subscriber." << std::endl;
            throw 0;
        }

        ISBridge *bridge = nullptr;
        if (sub_element && pub_element) // RTPS Only bridge
        {
            bridge = ISBridgeRTPS::configureBridge(bridge_element);
            if (bridge)
            {
                manager->addBridge(bridge);
            }
            else
            {
                std::cout << "Error loading RTPS bridge configuration. " << std::endl;
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
                            std::cout << "INFO: bridge_configuration is empty." << std::endl;
                            bridge = loadLib(nullptr); // Let's try to load without configuration.
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
                            listener->setTransformation(function_path);
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
}

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
                        std::cout << "INFO: bridge_configuration is empty." << std::endl;
                        bridge = loadLib(nullptr); // Let's try to load without configuration.
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
                    listener->setTransformation(function_path);
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

void ISManager::onTerminate()
{
    for (ISBridge* b : bridge)
    {
        if (b)
        {
            b->onTerminate();
        }
    }

    for (void* h : handle)
    {
        if(h) eProsimaCloseLibrary(h);
    }
}

bool ISManager::isActive(){
    return active;
}

ISManager::~ISManager()
{
    onTerminate();
    for (ISBridge* b : bridge)
    {
        delete b;
    }
    bridge.clear();
}
