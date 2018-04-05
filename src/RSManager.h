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

#include <vector>
#include <iostream>
#include "RSBridge.h"
#include <tinyxml2.h>

typedef RSBridge* (*loadbridgef_t)(tinyxml2::XMLElement *bridge_element);

class RSManager {
    std::vector<RSBridge*> bridge;
    std::vector<void*> handle;
    bool active;
    tinyxml2::XMLElement* _assignNextElement(tinyxml2::XMLElement *element, std::string name);
public:
    RSManager(std::string xml_file_path);
    ~RSManager();
    bool isActive();
    void addHandle(void* h);
    void addBridge(RSBridge* b);
    void onTerminate();
};

#endif // _RSMANAGER_H_
