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

#ifndef _ISSUBSCRIBER_H_
#define _ISSUBSCRIBER_H_

//#include <unordered_set>
#include <vector>
//#include <map>
//#include <string>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>
#include "ISBaseClass.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class ISSubscriber;
class ISBridge;

typedef ISSubscriber* (*createSubf_t)(ISBridge *bridge, const char* name,
                                      const std::vector<std::pair<std::string, std::string>> *config);

/** Base class for subscribers. Must know how to read from the origin protocol */
class ISSubscriber : public ISBaseClass
{
protected:
    std::vector<ISBridge*> mv_bridges;
public:
    ISSubscriber(const std::string &name) : ISBaseClass(name) { };
    virtual ~ISSubscriber() = default;
    virtual void addBridge(ISBridge* bridge);
    virtual void on_received_data(types::DynamicData* pData);

    // Forbid copy
    ISSubscriber(const ISSubscriber&) = delete;
    ISSubscriber& operator=(const ISSubscriber&) = delete;
};

#endif // _ISSUBSCRIBER_H_
