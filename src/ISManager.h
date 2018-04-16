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

#include <vector>
#include <iostream>
#include "ISBridge.h"
#include <tinyxml2.h>

typedef ISBridge* (*loadbridgef_t)(tinyxml2::XMLElement *bridge_element);

class ISManager {
    std::vector<ISBridge*> bridge;
    std::vector<void*> handle;
    bool active;
    tinyxml2::XMLElement* _assignNextElement(tinyxml2::XMLElement *element, std::string name);
public:
    ISManager(std::string xml_file_path);
    ~ISManager();
    bool isActive();
    void addHandle(void* h);
    void addBridge(ISBridge* b);
    void onTerminate();
};

#endif // _ISMANAGER_H_
