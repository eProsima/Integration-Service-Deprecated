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


#ifndef _ISBRIDGERTPS_H_
#define _ISBRIDGERTPS_H_

#include <string>

#include "ISBridge.h"
#include "GenericPubSubTypes.h"
#include "dynamicload/dynamicload.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
class RTPSBridge : public ISBridge
{
public:
    RTPSBridge(const std::string &name);
    virtual ~RTPSBridge();
};

#endif // _Header__SUBSCRIBER_H_
