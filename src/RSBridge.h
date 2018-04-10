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

#ifndef _RSBRIDGE_H_
#define _RSBRIDGE_H_

#include <fastrtps/fastrtps_fwd.h>
#include "GenericPubSubTypes.h"

#include "dynamicload/dynamicload.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

/**
 * Base class for Bridges. All implementation must inherit from it.
 */
class RSBridge
{
public:
    /**
     * This method will be called by RSManager when terminating the execution of the bridge.
     * Any handle, subscription, and resources that the bridge needed to work must be closed.
     */
    virtual void onTerminate() = 0;
    virtual ~RSBridge() = default;
};

typedef void (*userf_t)(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output);

#endif // _Header__SUBSCRIBER_H_
