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

#ifndef _IS_BRIDGE_H_
#define _IS_BRIDGE_H_

#include <unordered_set>
#include <vector>
#include <map>
#include <string>
#include "ISBaseClass.h"
#include "ISPublisher.h"
#include "ISSubscriber.h"

#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicDataFactory.h>

using namespace eprosima::fastrtps::types;

/**
 * Base class for Bridges. All implementation must inherit from it.
 */
class ISBridge : public ISBaseClass
{
protected:
    std::vector<ISSubscriber*> mv_subscriber;
    std::map<std::string, userf_t> mm_functionsNames;
    std::map<std::string, userdynf_t> mm_dynFunctionsNames;
    std::map<std::string, std::vector<std::string>> mm_functions;
    std::map<std::string, std::vector<std::string>> mm_dynFunctions;
    std::map<std::string, std::vector<ISPublisher*>> mm_publisher;
    std::map<ISPublisher*, std::string> mm_inv_publisher;
    std::unordered_set<ISBaseClass*> ms_subpubs;

    static std::string generateKeyPublisher(const std::string &sub, const std::string &funct)
    {
        return sub + "@" + funct;
    }
public:
    ISBridge(const std::string &name) : ISBaseClass(name) { };

    virtual ~ISBridge()
    {
        for (ISBaseClass* bc : ms_subpubs)
        {
            delete bc;
        }
    }

    /**
     * This method will be called by ISManager when terminating the execution of the bridge.
     * Any handle, subscription, and resources that the bridge needed to work must be closed.
     */
    virtual void onTerminate()
    {
        for (ISBaseClass* bc : ms_subpubs)
        {
            bc->onTerminate();
        }
    }

    virtual void addSubscriber(ISSubscriber *sub)
    {
        mv_subscriber.emplace_back(sub);
        ms_subpubs.emplace(sub);
        sub->addBridge(this);
    }

    virtual void addFunction(const std::string &sub, const std::string &fname, userf_t func)
    {
        mm_functionsNames[fname] = func;
        std::vector<std::string> &fnames = mm_functions[sub];
        fnames.push_back(fname);
    }

    virtual void addFunction(const std::string &sub, const std::string &fname, userdynf_t func)
    {
        mm_dynFunctionsNames[fname] = func;
        std::vector<std::string> &fnames = mm_dynFunctions[sub];
        fnames.push_back(fname);
    }

    virtual void addPublisher(const std::string &sub, const std::string &funcName, ISPublisher* pub)
    {
        std::string key = generateKeyPublisher(sub, funcName);
        std::vector<ISPublisher*> &pubs = mm_publisher[key];
        pubs.emplace_back(pub);
        mm_inv_publisher[pub] = key;
        ms_subpubs.emplace(pub);
        pub->setBridge(this);
    }

    virtual ISPublisher* removePublisher(ISPublisher* pub)
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

    virtual void on_received_data(const ISSubscriber *sub, SerializedPayload_t *data)
    {
        std::vector<std::string> funcNames = mm_functions[sub->getName()];
        for (std::string fName : funcNames)
        {
            userf_t function = mm_functionsNames[fName];
            SerializedPayload_t output;
            if (function)
            {
                function(data, &output);
            }
            else
            {
                output.copy(data, false);
            }
            std::vector<ISPublisher*> pubs = mm_publisher[generateKeyPublisher(sub->getName(), fName)];
            for (ISPublisher* pub : pubs)
            {
                pub->publish(&output);
            }
        }
    }

    virtual void on_received_data(const ISSubscriber *sub, DynamicData *data)
    {
        std::vector<std::string> funcNames = mm_dynFunctions[sub->getName()];
        for (std::string fName : funcNames)
        {
            std::vector<ISPublisher*> pubs = mm_publisher[generateKeyPublisher(sub->getName(), fName)];
            for (ISPublisher* pub : pubs)
            {
                userdynf_t function = mm_dynFunctionsNames[fName];

                DynamicData* output;
                //DynamicData output; // TODO nueva instancia desde el tipo del publisher (añadir mapeo función)
                if (function)
                {
                    TopicDataType *output_type = pub->output_type;
                    DynamicPubSubType *pst = dynamic_cast<DynamicPubSubType*>(output_type);
                    if (pst == nullptr)
                    {
                        //LOG_ERROR("Trying to call dynamic function with static data type: " << fName.c_str() << ", " << output_type->getName());
                        continue;
                    }
                    else
                    {
                        output = DynamicDataFactory::GetInstance()->CreateData(pst->GetDynamicType());
                        function(data, output);
                    }
                }
                else
                {
                    output = DynamicDataFactory::GetInstance()->CreateCopy(data);
                    //output->copy(data, false);
                }

                pub->publish(output);
            }
        }
    }

    // Forbid copy
    ISBridge(const ISBridge&) = delete;
    ISBridge& operator=(const ISBridge&) = delete;
};

typedef ISBridge* (*createBridgef_t)(const char* name, const std::vector<std::pair<std::string, std::string>> *config);


inline void ISSubscriber::addBridge(ISBridge* bridge){
    mv_bridges.push_back(bridge);
}

inline void ISSubscriber::on_received_data(SerializedPayload_t* payload)
{
    for (ISBridge* bridge : mv_bridges)
    {
        bridge->on_received_data(this, payload);
    }
}

inline void ISSubscriber::on_received_data(DynamicData* data)
{
    for (ISBridge* bridge : mv_bridges)
    {
        bridge->on_received_data(this, data);
    }
}

inline ISBridge* ISPublisher::setBridge(ISBridge *bridge)
{
    ISBridge *old = mb_bridge;
    mb_bridge = bridge;
    if (old != nullptr && old != bridge)
    {
        old->removePublisher(this);
    }
    return old;
}

#endif // _IS_BRIDGE_H_
