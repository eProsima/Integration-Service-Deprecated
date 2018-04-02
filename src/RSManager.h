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


#ifndef _RSMANAGER_H_
#define _RSMANAGER_H_

#include <iostream>
#include "RSBridgeNGSIv2ToFastRTPS.h"
#include "RSBridgeFastRTPSToNGSIv2.h"
#include "RSBridge.h"
#include <tinyxml2.h>


class RSManager {
    std::vector<RSBridge> bridge;
    std::vector<RSBridgeNGSIv2ToFastRTPS> bridgeNGSIv2ToFastRTPS;
    std::vector<RSBridgeFastRTPSToNGSIv2> bridgeFastRTPSToNGSIv2;
    bool active;
public:
    RSManager(std::string xml_file_path);
    void LoadFastRTPSBridge(tinyxml2::XMLElement *bridge_element);
    void LoadNGSIv2ToFastRTPSBridge(tinyxml2::XMLElement *bridge_element);
    void LoadFastRTPSToNGSIv2Bridge(tinyxml2::XMLElement *bridge_element);
    ~RSManager(){};
    bool isActive();
    tinyxml2::XMLElement* _assignNextElement(tinyxml2::XMLElement *element, std::string name);

};

#endif // _RSMANAGER_H_
