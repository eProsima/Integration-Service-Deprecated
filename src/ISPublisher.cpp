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

#include "ISPublisher.h"
#include "ISBridge.h"
#include "log/ISLog.h"

ISPublisher::ISPublisher(const std::string &name, bool bDynamicType)
    : ISBaseClass(name)
    , mb_bridge(nullptr)
    , m_bDynamicType(bDynamicType)
{
}

bool ISPublisher::publish(types::DynamicData* data)
{
    LOG_ERROR("ISPublisher: Undefinded publish method");
    return false;
}

bool ISPublisher::publish(SerializedPayload_t* data)
{
    LOG_ERROR("ISPublisher: Undefinded publish method");
    return false;
}

ISBridge* ISPublisher::setBridge(ISBridge *bridge)
{
    ISBridge *old = mb_bridge;
    mb_bridge = bridge;
    if (old != nullptr && old != bridge)
    {
        old->removePublisher(this);
    }
    return old;
}