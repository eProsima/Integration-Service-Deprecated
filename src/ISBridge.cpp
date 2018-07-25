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

#include "ISBridge.h"
#include "ISSubscriber.h"
#include "ISPublisher.h"
#include "Log/ISLog.h"
#include <fastrtps/types/DynamicDataFactory.h>

void ISBridge::onTerminate()
{
    for (ISBaseClass* bc : ms_subpubs)
    {
        bc->onTerminate();
    }
}

ISBridge::~ISBridge()
{
    for (ISBaseClass* bc : ms_subpubs)
    {
        delete bc;
    }
}

void ISBridge::addSubscriber(ISSubscriber *sub)
{
    mv_subscriber.emplace_back(sub);
    ms_subpubs.emplace(sub);
    sub->addBridge(this);
}

void ISBridge::addFunction(const std::string &sub, const std::string &fname, void* func, bool dynamicInput,
    bool dynamicOutput)
{
    if (fname.length() > 0)
    {
        if (dynamicInput)
        {
            if (dynamicOutput)
            {
                mm_dynamic_dynamicFunctionsNames[fname] = (userdyn_dynf_t)func;
            }
            else
            {
                mm_dynamic_rawFunctionsNames[fname] = (userdyn_rawf_t)func;
            }
        }
        else
        {
            if (dynamicOutput)
            {
                mm_raw_dynamicFunctionsNames[fname] = (userraw_dynf_t)func;
            }
            else
            {
                mm_raw_rawFunctionsNames[fname] = (userraw_rawf_t)func;
            }
        }
        std::vector<std::string> &fnames = mm_functions[sub];
        fnames.push_back(fname);
    }
}

void ISBridge::addPublisher(const std::string &sub, const std::string &funcName, ISPublisher* pub)
{
    std::string key = generateKeyPublisher(sub, funcName);
    std::vector<ISPublisher*> &pubs = mm_publisher[key];
    pubs.emplace_back(pub);
    //mm_publisher.emplace(key, pub);
    mm_inv_publisher[pub] = key;
    ms_subpubs.emplace(pub);
    pub->setBridge(this);
}

bool ISBridge::exists_transform_function(const std::string& name, bool bDynamicInput)
{
    if (bDynamicInput)
    {
        return mm_dynamic_dynamicFunctionsNames.find(name) != mm_dynamic_dynamicFunctionsNames.end() ||
            mm_dynamic_rawFunctionsNames.find(name) != mm_dynamic_rawFunctionsNames.end();
    }
    else
    {
        return mm_raw_dynamicFunctionsNames.find(name) != mm_raw_dynamicFunctionsNames.end() ||
            mm_raw_rawFunctionsNames.find(name) != mm_raw_rawFunctionsNames.end();
    }
}

ISPublisher* ISBridge::removePublisher(ISPublisher* pub)
{
    std::string key = mm_inv_publisher[pub];
    mm_inv_publisher.erase(pub);
    std::vector<ISPublisher*> pubs = mm_publisher[key];
    for (auto it = pubs.begin(); it != pubs.end(); it++)
    {
        if (*it == pub)
        {
            pubs.erase(it);
            break;
        }
    }
    ms_subpubs.erase(pub);
    return pub;
}

bool ISBridge::on_received_data(const ISSubscriber *sub, types::DynamicData* pInputData)
{
    std::vector<std::string> funcNames = mm_functions[sub->getName()];
    for (std::string fName : funcNames)
    {
        // If there is a transformation function
        bool bDynamicOutput = true;
        if (exists_transform_function(fName, true))
        {
            void* pOutputData = run_transform_function(fName, pInputData, true, bDynamicOutput);
            if (pOutputData != nullptr)
            {
                std::vector<ISPublisher*> pubs = mm_publisher[generateKeyPublisher(sub->getName(), fName)];
                for (ISPublisher* pub : pubs)
                {
                    if (pub->getDynamicType() == bDynamicOutput)
                    {
                        if (bDynamicOutput)
                        {
                            pub->publish((types::DynamicData*)pOutputData);
                            types::DynamicDataFactory::GetInstance()->DeleteData((types::DynamicData*)pOutputData);
                        }
                        else
                        {
                            pub->publish((SerializedPayload_t*)pOutputData);
                            delete pOutputData;
                        }
                    }
                    else
                    {
                        LOG_ERROR("Incompatible publisher. Check the configuration with the dynamic types");
                    }
                }
            }
            else
            {
                LOG_ERROR("Transformation function hasn't sent any data");
            }
        }
        // If there isn't any transformation function, just send the same data.
        else
        {
            std::vector<ISPublisher*> pubs = mm_publisher[generateKeyPublisher(sub->getName(), fName)];
            for (ISPublisher* pub : pubs)
            {
                if (pub->getDynamicType())
                {
                    pub->publish(pInputData);
                }
                else
                {
                    types::DynamicPubSubType pubsubType;
                    uint32_t payloadSize = static_cast<uint32_t>(pubsubType.getSerializedSizeProvider(pInputData)());
                    SerializedPayload_t payload(payloadSize);
                    pubsubType.serialize(pInputData, &payload);
                    pub->publish(&payload);
                }
            }
        }
    }
    return true;
}

bool ISBridge::on_received_data(const ISSubscriber *sub, SerializedPayload_t* pBuffer)
{
    std::vector<std::string> funcNames = mm_functions[sub->getName()];
    for (std::string fName : funcNames)
    {
        // If there is a transformation function
        bool bDynamicOutput = false;
        if (exists_transform_function(fName, false))
        {
            void* pOutputData = run_transform_function(fName, pBuffer, false, bDynamicOutput);
            if (pOutputData != nullptr)
            {
                std::vector<ISPublisher*> pubs = mm_publisher[generateKeyPublisher(sub->getName(), fName)];
                for (ISPublisher* pub : pubs)
                {
                    if (pub->getDynamicType() == bDynamicOutput)
                    {
                        if (bDynamicOutput)
                        {
                            pub->publish((types::DynamicData*)pOutputData);
                            types::DynamicDataFactory::GetInstance()->DeleteData((types::DynamicData*)pOutputData);
                        }
                        else
                        {
                            pub->publish((SerializedPayload_t*)pOutputData);
                            delete pOutputData;
                        }
                    }
                    else
                    {
                        LOG_ERROR("Incompatible publisher. Check the configuration with the dynamic types");
                    }
                }
            }
            else
            {
                LOG_ERROR("Transformation function hasn't sent any data");
            }
        }
        // If there isn't any transformation function, just send the same data.
        else
        {
            std::vector<ISPublisher*> pubs = mm_publisher[generateKeyPublisher(sub->getName(), fName)];
            for (ISPublisher* pub : pubs)
            {
                if (pub->getDynamicType())
                {
                    LOG_ERROR("Configuration error. Check the configuration with dynamic types");
                }
                else
                {
                    pub->publish(pBuffer);
                }
            }
        }
    }
    return true;
}

void* ISBridge::run_transform_function(const std::string& name, void* inputData, bool dynamicInput, bool& dynamicOutput)
{
    if (dynamicInput)
    {
        auto dyndynIt = mm_dynamic_dynamicFunctionsNames.find(name);
        if (dyndynIt != mm_dynamic_dynamicFunctionsNames.end())
        {
            dynamicOutput = true;
            void* outputData(nullptr);
            dyndynIt->second((types::DynamicData*)inputData, (types::DynamicData**)&outputData);
            return outputData;
        }
        else
        {
            auto dyndynIt = mm_dynamic_rawFunctionsNames.find(name);
            if (dyndynIt != mm_dynamic_rawFunctionsNames.end())
            {
                dynamicOutput = false;
                void* outputData(nullptr);
                dyndynIt->second((types::DynamicData*)inputData, (SerializedPayload_t**)&outputData);
                return outputData;
            }
        }
    }
    else
    {
        auto dyndynIt = mm_raw_dynamicFunctionsNames.find(name);
        if (dyndynIt != mm_raw_dynamicFunctionsNames.end())
        {
            dynamicOutput = true;
            void* outputData(nullptr);
            dyndynIt->second((SerializedPayload_t*)inputData, (types::DynamicData**)&outputData);
            return outputData;
        }
        else
        {
            auto dyndynIt = mm_raw_rawFunctionsNames.find(name);
            if (dyndynIt != mm_raw_rawFunctionsNames.end())
            {
                dynamicOutput = false;
                void* outputData(nullptr);
                dyndynIt->second((SerializedPayload_t*)inputData, (SerializedPayload_t**)&outputData);
                return outputData;
            }
        }
    }
    return nullptr;
}