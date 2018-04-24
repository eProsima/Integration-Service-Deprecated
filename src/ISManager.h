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


#ifndef _ISMANAGER_H_
#define _ISMANAGER_H_

#include <map>
#include <iostream>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include "ISBridge.h"
#include "dynamicload/dynamicload.h"
#include "xmlUtils.h"

typedef ISBridge* (*loadbridgef_t)(const char *config);

class ISManager {
    std::map<std::string, ISBridge*> bridges;
    std::map<std::string, ISSubscriber*> subscribers;
    std::map<std::string, ISPublisher*> publishers;
    std::map<std::string, void*> handles;
    bool active;
    void parseProperties(tinyxml2::XMLElement *parent_element,
                         std::vector<std::pair<std::string, std::string>> &props);
public:
    ISManager(const std::string &xml_file_path);
    ~ISManager();
    bool isActive();
    void addBridge(ISBridge* b);
    void addPublisher(ISPublisher* p);
    void addSubscriber(ISSubscriber* s);
    void addPublisher(const std::string &part_name, ISPublisher* p);
    void addSubscriber(const std::string &part_name, ISSubscriber* s);
    void loadParticipant(tinyxml2::XMLElement *participant_element);
    void loadSubscriber(ParticipantAttributes &part_attrs, tinyxml2::XMLElement *subscriber_element);
    void loadPublisher(ParticipantAttributes &part_attrs, tinyxml2::XMLElement *publisher_element);
    void loadBridge(tinyxml2::XMLElement *bridge_element);
    void loadConnector(tinyxml2::XMLElement *connector_element);
    void onTerminate();
    void* getLibraryHandle(const std::string &libpath);
    std::string getEndPointName(const std::string &partName, const std::string &epName)
    {
        return partName + "." + epName;
    }
};

#endif // _ISMANAGER_H_
