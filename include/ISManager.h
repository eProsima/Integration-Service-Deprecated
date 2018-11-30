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
#include <tinyxml2.h>
#include "log/ISLog.h"
#include "ISBridge.h"
#include "dynamicload/dynamicload.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/participant/ParticipantListener.h>

typedef ISBridge* (*loadbridgef_t)(const char *config);

class MyParticipantListener : public ParticipantListener
{
public:
    virtual void onParticipantDiscovery(Participant* /*p*/, ParticipantDiscoveryInfo info)
    {
        if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            LOG_INFO("Participant discovered " << info.info.m_participantName);
        }
        else if (info.status == ParticipantDiscoveryInfo::REMOVED_PARTICIPANT)
        {
            LOG_INFO("Participant removed " << info.info.m_participantName);
        }
        else if (info.status == ParticipantDiscoveryInfo::DROPPED_PARTICIPANT)
        {
            LOG_INFO("Participant dropped " << info.info.m_participantName);
        }
        else if (info.status == ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT)
        {
            LOG_INFO("Participant changed QOS " << info.info.m_participantName);
        }
    }
};

class ISManager
{
    std::map<std::string, ISBridge*> bridges;
    std::map<std::string, ISReader*> readers;
    std::map<std::string, ISWriter*> writers;
    std::map<std::string, void*> handles;
    std::map<std::string, typef_t> typesLibs;
    std::map<std::string, Participant*> rtps_participants;
    std::vector<typef_t> defaultTypesLibs;
    std::map<std::pair<std::string, std::string>, TopicDataType*> data_types;
    bool active;
    void parseProperties(tinyxml2::XMLElement *parent_element,
        std::vector<std::pair<std::string, std::string>> &props);
    MyParticipantListener myParticipantListener;
    TopicDataType* getTopicDataType(const std::string &type);
public:
    ISManager(const std::string &xml_file_path);
    ~ISManager();
    bool isActive();
    void addBridge(ISBridge* b);
    void addWriter(ISWriter* p);
    void addReader(ISReader* s);
    void addWriter(const std::string &part_name, ISWriter* p);
    void addReader(const std::string &part_name, ISReader* s);
    void loadISTypes(tinyxml2::XMLElement *is_types_element);
    void loadProfiles(tinyxml2::XMLElement *profiles);
    void loadDynamicTypes(tinyxml2::XMLElement *types);
    Participant* getParticipant(const std::string &name);
    void createReader(Participant* participant, const std::string &participant_profile, const std::string &name);
    void createWriter(Participant* participant, const std::string &participant_profile, const std::string &name);
    void loadBridge(tinyxml2::XMLElement *bridge_element);
    void loadConnector(tinyxml2::XMLElement *connector_element);
    void onTerminate();
    void* getLibraryHandle(const std::string &libpath);
    void* getTypeHandle(const std::string &libpath);
    std::string getEndPointName(const std::string &partName, const std::string &epName)
    {
        return partName + "." + epName;
    }
};

#endif // _ISMANAGER_H_
