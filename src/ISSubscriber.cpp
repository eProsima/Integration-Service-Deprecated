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

#include "ISSubscriber.h"
#include "ISBridge.h"

void ISSubscriber::addBridge(ISBridge* bridge)
{
    mv_bridges.push_back(bridge);
}

void ISSubscriber::on_received_data(types::DynamicData* pInputData)
{
    for (ISBridge* bridge : mv_bridges)
    {
        bridge->on_received_data(this, pInputData);
    }
}
