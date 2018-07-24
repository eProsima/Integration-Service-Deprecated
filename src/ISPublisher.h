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

#ifndef _ISPUBLISHER_H_
#define _ISPUBLISHER_H_

//#include <unordered_set>
//#include <vector>
//#include <map>
//#include <string>
#include "ISBaseClass.h"
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>
//#include <fastrtps/types/DynamicPubSubType.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class ISPublisher;
class ISBridge;

typedef ISPublisher* (*createPubf_t)(ISBridge *bridge, const char* name,
                                     const std::vector<std::pair<std::string, std::string>> *config);

/** Base class for publishers. Must know how to write into the destination protocol */
class ISPublisher : public ISBaseClass
{
protected:
    ISBridge *mb_bridge;
public:
    ISPublisher(const std::string &name);
    virtual ~ISPublisher() = default;
    virtual bool publish(types::DynamicData * data) { return false; };
    virtual ISBridge* setBridge(ISBridge *bridge);

    // Forbid copy
    ISPublisher(const ISPublisher&) = delete;
    ISPublisher& operator=(const ISPublisher&) = delete;
};

#endif // _ISPUBLISHER_H_
