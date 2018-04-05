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

void loadNGSIv2Bridge(RSManager *manager, tinyxml2::XMLElement *bridge_element);
void loadUnidirectional(RSManager *manager, tinyxml2::XMLElement *bridge_element);
void loadBidirectional(RSManager *manager, tinyxml2::XMLElement *bridge_element, std::string nodeA, std::string nodeB);

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

    for (auto child = bridge_element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
    {
        tinyxml2::XMLElement *current_element = _assignNextElement(child, "bridge_type");
        const char* bridge_type = current_element->GetText();
        if (strncmp(bridge_type, "fastrtps", 8) == 0
            || strncmp(bridge_type, "ros2", 4) == 0
            || strncmp(bridge_type, "unidirectional", 14) == 0)
        {
            loadUnidirectional(this, child);
        }
        else if (strncmp(bridge_type, "ngsiv2", 6) == 0)
        {
            loadNGSIv2Bridge(this, child);
        }
        else if (strncmp(bridge_type, "bidirectional", 13) == 0)
        {
            loadBidirectional(this, child, "nodeA", "nodeB");
        }
    }

    if (bridge.size() > 0)
    {
        active = true;
    }
}

void RSManager::addBridge(RSBridge* b)
{
    bridge.emplace_back(b);
    active = true;
}

void RSManager::addHandle(void* h)
{
    handle.emplace_back(h);
}

void loadUnidirectional(RSManager *manager, tinyxml2::XMLElement *bridge_element)
{
    try
    {
        void* handle;

        tinyxml2::XMLElement *lib_element = bridge_element->FirstChildElement("bridge_library");

        if(!lib_element)
        {
            throw 0;
        }

        const char* file_path = lib_element->GetText();
        handle = eProsimaLoadLibrary(file_path);
        manager->addHandle(handle);
        loadbridgef_t loadLib = (loadbridgef_t)eProsimaGetProcAddress(handle, "createBridge");

        manager->addBridge(loadLib(bridge_element));
    }
    catch (int e_code){
        std::cout << "Error ocurred while loading bridge library " << e_code << std::endl;
    }
}

void loadNGSIv2Bridge(RSManager *manager, tinyxml2::XMLElement *bridge_element)
{
    loadBidirectional(manager, bridge_element, "ros2", "ngsiv2");
}

void loadBidirectional(RSManager *manager, tinyxml2::XMLElement *bridge_element, std::string nodeA, std::string nodeB)
{
    try
    {
        void* handle1;
        void* handle2;
        std::string lib1 = "bridge_library_" + nodeA;
        std::string lib2 = "bridge_library_" + nodeB;

        tinyxml2::XMLElement *lib1_element = bridge_element->FirstChildElement(lib1.c_str());
        tinyxml2::XMLElement *lib2_element = bridge_element->FirstChildElement(lib2.c_str());

        if(!lib1_element || !lib2_element)
        {
            throw 0;
        }

        const char* file_path1 = lib1_element->GetText();
        const char* file_path2 = lib2_element->GetText();
        handle1 = eProsimaLoadLibrary(file_path1);
        handle2 = eProsimaLoadLibrary(file_path2);
        manager->addHandle(handle1);
        manager->addHandle(handle2);
        loadbridgef_t loadLib1 = (loadbridgef_t)eProsimaGetProcAddress(handle1, "createBridge");
        loadbridgef_t loadLib2 = (loadbridgef_t)eProsimaGetProcAddress(handle2, "createBridge");

        manager->addBridge(loadLib1(bridge_element));
        manager->addBridge(loadLib2(bridge_element));
    }
    catch (int e_code){
        std::cout << "Error ocurred while loading bridge library " << e_code << std::endl;
    }
}

tinyxml2::XMLElement* RSManager::_assignNextElement(tinyxml2::XMLElement *element, std::string name){
    if (!element->FirstChildElement(name.c_str())){
        throw 0;
    }
    return element->FirstChildElement(name.c_str());
}

void RSManager::onTerminate()
{
    for (RSBridge* b : bridge)
    {
        b->onTerminate();
    }

    for (void* h : handle)
    {
        if(h) eProsimaCloseLibrary(h);
    }
}

bool RSManager::isActive(){
    return active;
}

RSManager::~RSManager()
{
    onTerminate();
    for (RSBridge* b : bridge)
    {
        delete b;
    }
}
