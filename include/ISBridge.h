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
#include "ISWriter.h"
#include "ISReader.h"

#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/DynamicDataFactory.h>

using namespace eprosima::fastrtps::types;

/**
 * Base class for Bridges. All implementation must inherit from it.
 */
class ISBridge : public ISBaseClass
{
protected:
    std::vector<ISReader*> mv_reader;
    std::map<std::string, userf_t> mm_functionsNames;
    std::map<std::string, userdynf_t> mm_dynFunctionsNames;
    std::map<std::string, std::vector<std::string>> mm_functions;
    std::map<std::string, std::vector<std::string>> mm_dynFunctions;
    std::map<std::string, std::vector<ISWriter*>> mm_writer;
    std::map<ISWriter*, std::string> mm_inv_writer;
    std::unordered_set<ISBaseClass*> ms_subpubs;
    std::map<std::string, SerializedPayload_t*> mm_staticPayload; // Key is the writer
    std::map<std::string, DynamicData*> mm_dynamicData; // Key is the writer
    std::map<std::string, std::mutex*> mm_mutex; // Key is the writer

    static std::string generateKeyWriter(const std::string &sub, const std::string &funct)
    {
        return sub + "@" + funct;
    }
public:
    ISBridge(const std::string &name) : ISBaseClass(name) { };

    virtual ~ISBridge()
    {
        for (auto it : mm_staticPayload)
        {
            delete it.second;
        }
        for (auto it : mm_dynamicData)
        {
            DynamicDataFactory::GetInstance()->DeleteData(it.second);
        }
        for (auto it : mm_mutex)
        {
            delete it.second;
        }
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
        ISBaseClass::onTerminate();

        for (ISReader* sub : mv_reader)
        {
            sub->onTerminate();
        }

        for (auto pub_pair : mm_writer)
        {
            for (ISWriter* pub : pub_pair.second)
            {
                pub->onTerminate();
            }
        }

        for (ISBaseClass* bc : ms_subpubs)
        {
            bc->onTerminate();
        }
    }

    virtual void addReader(ISReader *sub)
    {
        mv_reader.emplace_back(sub);
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

    virtual void addWriter(const std::string &sub, const std::string &funcName, ISWriter* pub)
    {
        std::string key = generateKeyWriter(sub, funcName);
        std::vector<ISWriter*> &pubs = mm_writer[key];
        pubs.emplace_back(pub);
        mm_inv_writer[pub] = key;
        ms_subpubs.emplace(pub);
        pub->setBridge(this);
    }

    virtual ISWriter* removeWriter(ISWriter* pub)
    {
        std::string key = mm_inv_writer[pub];
        mm_inv_writer.erase(pub);
        std::vector<ISWriter*> pubs = mm_writer[key];
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

    virtual void on_received_data(const ISReader *sub, SerializedPayload_t *data)
    {
        if (!isTerminating())
        {
            std::vector<std::string> funcNames = mm_functions[sub->getName()];
            for (std::string fName : funcNames)
            {
                std::string keyPub = generateKeyWriter(sub->getName(), fName);
                userf_t function = mm_functionsNames[fName];

                if (function)
                {
                    std::mutex* mutex = mm_mutex[keyPub];
                    if (mutex == nullptr)
                    {
                        mutex = new std::mutex();
                        mm_mutex[keyPub] = mutex;
                    }
                    std::unique_lock<std::mutex> scopedLock(*mutex);
                    SerializedPayload_t* output = mm_staticPayload[keyPub];
                    if (output == nullptr)
                    {
                        output = new SerializedPayload_t();
                        mm_staticPayload[keyPub] = output;
                    }

                    function(data, output);
                    std::vector<ISWriter*> pubs = mm_writer[keyPub];
                    for (ISWriter* pub : pubs)
                    {
                        if (!pub->isTerminating())
                        {
                            pub->write(output);
                        }
                    }
                }
                else
                {
                    //output.copy(data, false);
                    std::vector<ISWriter*> pubs = mm_writer[keyPub];
                    for (ISWriter* pub : pubs)
                    {
                        if (!pub->isTerminating())
                        {
                            pub->write(data);
                        }
                    }
                }
            }
        }
    }

    virtual void on_received_data(const ISReader *sub, DynamicData *data)
    {
        if (!isTerminating())
        {
            std::vector<std::string> funcNames = mm_dynFunctions[sub->getName()];
            if (funcNames.empty())
            {
                std::string keyPub = generateKeyWriter(sub->getName(), "");
                std::vector<ISWriter*> pubs = mm_writer[keyPub];
                for (ISWriter* pub : pubs)
                {
                    if (!pub->isTerminating())
                    {
                        pub->write(data);
                        /*
                        DynamicData* output = mm_dynamicData[keyPub];
                        if (output == nullptr)
                        {
                            output = DynamicDataFactory::GetInstance()->CreateCopy(data);
                            mm_dynamicData[keyPub] = output;
                        }
                        pub->write(output);
                        //DynamicDataFactory::GetInstance()->DeleteData(output);
                        */
                    }
                }
            }
            else
            {
                for (std::string fName : funcNames)
                {
                    std::string keyPub = generateKeyWriter(sub->getName(), fName);
                    std::vector<ISWriter*> pubs = mm_writer[keyPub];
                    for (ISWriter* pub : pubs)
                    {
                        if (pub->isTerminating()) continue;

                        userdynf_t function = mm_dynFunctionsNames[fName];

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
                                std::mutex* mutex = mm_mutex[keyPub];
                                if (mutex == nullptr)
                                {
                                    mutex = new std::mutex();
                                    mm_mutex[keyPub] = mutex;
                                }
                                std::unique_lock<std::mutex> scopedLock(*mutex);
                                DynamicData* output = mm_dynamicData[keyPub];
                                if (output == nullptr)
                                {
                                    output = DynamicDataFactory::GetInstance()->CreateData(pst->GetDynamicType());
                                    mm_dynamicData[keyPub] = output;
                                }
                                function(data, output);
                                pub->write(output);
                                //DynamicDataFactory::GetInstance()->DeleteData(output);
                            }
                        }
                        else
                        {
                            //output = DynamicDataFactory::GetInstance()->CreateCopy(data);
                            //output->copy(data, false);
                            pub->write(data);
                        }
                    }
                }
            }
        }
    }

    // Forbid copy
    ISBridge(const ISBridge&) = delete;
    ISBridge& operator=(const ISBridge&) = delete;
};

typedef ISBridge* (*createBridgef_t)(const char* name, const std::vector<std::pair<std::string, std::string>> *config);


inline void ISReader::addBridge(ISBridge* bridge){
    mv_bridges.push_back(bridge);
}

inline void ISReader::on_received_data(SerializedPayload_t* payload)
{
    if (!isTerminating())
    {
        for (ISBridge* bridge : mv_bridges)
        {
            if (!bridge->isTerminating())
            {
                bridge->on_received_data(this, payload);
            }
        }
    }
}

inline void ISReader::on_received_data(DynamicData* data)
{
    if (!isTerminating())
    {
        for (ISBridge* bridge : mv_bridges)
        {
            if (!bridge->isTerminating())
            {
                bridge->on_received_data(this, data);
            }
        }
    }
}

inline ISBridge* ISWriter::setBridge(ISBridge *bridge)
{
    ISBridge *old = mb_bridge;
    mb_bridge = bridge;
    if (old != nullptr && old != bridge)
    {
        old->removeWriter(this);
    }
    return old;
}

#endif // _IS_BRIDGE_H_
