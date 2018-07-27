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

#ifndef _ISBRIDGE_H_
#define _ISBRIDGE_H_

#include <unordered_set>
#include <vector>
#include <map>
#include <string>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/types/DynamicData.h>
#include <fastrtps/types/DynamicPubSubType.h>
#include "ISBaseClass.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

typedef void (*userdyn_dynf_t)(types::DynamicData* input_data, types::DynamicData** output_data);
typedef void (*userraw_dynf_t)(SerializedPayload_t* input_data, types::DynamicData** output_data);
typedef void (*userdyn_rawf_t)(types::DynamicData* input_data, SerializedPayload_t* output_data);
typedef void (*userraw_rawf_t)(SerializedPayload_t* input_data, SerializedPayload_t* output_data);
typedef TopicDataType* (*typef_t)(const char *name);

class ISPublisher;
class ISSubscriber;
class ISBridge;

typedef ISBridge* (*createBridgef_t)(const char* name, const std::vector<std::pair<std::string, std::string>> *config);
typedef ISPublisher* (*createPubf_t)(ISBridge *bridge, const char* name,
                                     const std::vector<std::pair<std::string, std::string>> *config);
typedef ISSubscriber* (*createSubf_t)(ISBridge *bridge, const char* name,
                                      const std::vector<std::pair<std::string, std::string>> *config);

/**
 * Base class for Bridges. All implementation must inherit from it.
 */
class ISBridge : public ISBaseClass
{
protected:
    types::DynamicPubSubType m_pubSubType;
    std::vector<ISSubscriber*> mv_subscriber;
    std::map<std::string, userdyn_dynf_t> mm_dynamic_dynamicFunctionsNames;
    std::map<std::string, userdyn_rawf_t> mm_dynamic_rawFunctionsNames;
    std::map<std::string, userraw_dynf_t> mm_raw_dynamicFunctionsNames;
    std::map<std::string, userraw_rawf_t> mm_raw_rawFunctionsNames;
    std::map<std::string, std::vector<std::string>> mm_functions;
    std::map<std::string, std::vector<ISPublisher*>> mm_publisher;
    std::map<ISPublisher*, std::string> mm_inv_publisher;
    std::unordered_set<ISBaseClass*> ms_subpubs;

    //userf_t *transformation;
    std::string generateKeyPublisher(const std::string &sub, const std::string &funct)
    {
        return sub + "@" + funct;
    }

    bool exists_transform_function(const std::string& name, bool bDynamicInput);
    void* run_transform_function(const std::string& name, void* inputData, bool dynamicInput, bool& dynamicOutput);
public:
    ISBridge(const std::string &name) : ISBaseClass(name){ };
    /**
     * This method will be called by ISManager when terminating the execution of the bridge.
     * Any handle, subscription, and resources that the bridge needed to work must be closed.
     */
    virtual ~ISBridge();
    virtual void addSubscriber(ISSubscriber *sub);
    virtual void addFunction(const std::string &sub, const std::string &fname, void* func, bool dynamicInput, bool dynamicOutput);
    virtual void addPublisher(const std::string &sub, const std::string &funcName, ISPublisher* pub);
    virtual ISPublisher* removePublisher(ISPublisher* pub);
    virtual bool on_received_data(const ISSubscriber *sub, types::DynamicData* pData);
    virtual bool on_received_data(const ISSubscriber *sub, SerializedPayload_t* pBuffer);
    virtual void onTerminate() override;

    // Forbid copy
    ISBridge(const ISBridge&) = delete;
    ISBridge& operator=(const ISBridge&) = delete;
};

#endif // _Header__SUBSCRIBER_H_
