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

#include "GenericPubSubTypes.h"

typedef void (*userf_t)(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output);
typedef TopicDataType* (*typef_t)(const char *name);

class ISPublisher;
class ISSubscriber;
class ISBridge;

typedef ISBridge* (*createBridgef_t)(const char* name, const std::vector<std::pair<std::string, std::string>> *config);
typedef ISPublisher* (*createPubf_t)(ISBridge *bridge, const char* name,
                                     const std::vector<std::pair<std::string, std::string>> *config);
typedef ISSubscriber* (*createSubf_t)(ISBridge *bridge, const char* name,
                                      const std::vector<std::pair<std::string, std::string>> *config);

class ISBaseClass
{
protected:
    std::string name;
    virtual void setName(const std::string &name) { this->name = name; }
public:
    ISBaseClass(const std::string &name) : name(name) {};
    virtual const std::string& getName() const { return name; }
    virtual void onTerminate() {};
    virtual ~ISBaseClass() = default;
};

/**
 * Base class for Bridges. All implementation must inherit from it.
 */
class ISBridge : public ISBaseClass
{
protected:
    std::vector<ISSubscriber*> mv_subscriber;
    std::map<std::string, userf_t> mm_functionsNames;
    std::map<std::string, std::vector<std::string>> mm_functions;
    std::map<std::string, std::vector<ISPublisher*>> mm_publisher;
    std::map<ISPublisher*, std::string> mm_inv_publisher;
    std::unordered_set<ISBaseClass*> ms_subpubs;

    static std::string generateKeyPublisher(const std::string &sub, const std::string &funct)
    {
        return sub + "@" + funct;
    }
public:
    ISBridge(const std::string &name) : ISBaseClass(name) { };
    /**
     * This method will be called by ISManager when terminating the execution of the bridge.
     * Any handle, subscription, and resources that the bridge needed to work must be closed.
     */
    virtual ~ISBridge() {};
    virtual void addSubscriber(ISSubscriber *sub) = 0;
    virtual void addFunction(const std::string &sub, const std::string &fname, userf_t func) = 0;
    virtual void addPublisher(const std::string &sub, const std::string &funcName, ISPublisher* pub) = 0;
    virtual ISPublisher* removePublisher(ISPublisher* pub) = 0;
    virtual void on_received_data(const ISSubscriber *sub, SerializedPayload_t *data) = 0;

    // Forbid copy
    ISBridge(const ISBridge&) = delete;
    ISBridge& operator=(const ISBridge&) = delete;
};

/** Base class for publishers. Must know how to write into the destination protocol */
class ISPublisher : public ISBaseClass
{
protected:
    ISBridge *mb_bridge;
public:
    ISPublisher(const std::string &name) : ISBaseClass(name), mb_bridge(nullptr) { };
    virtual ~ISPublisher() = default;
    virtual bool publish(SerializedPayload_t* /*data*/) { return false; };

    virtual ISBridge* setBridge(ISBridge *bridge)
    {
        ISBridge *old = mb_bridge;
        mb_bridge = bridge;
        if (old && old != bridge)
        {
            old->removePublisher(this);
        }
        return old;
    }

    // Forbid copy
    ISPublisher(const ISPublisher&) = delete;
    ISPublisher& operator=(const ISPublisher&) = delete;
};

/** Base class for subscribers. Must know how to read from the origin protocol */
class ISSubscriber : public ISBaseClass
{
protected:
    std::vector<ISBridge*> mv_bridges;
public:
    ISSubscriber(const std::string &name) : ISBaseClass(name) { };
    virtual ~ISSubscriber() = default;
    virtual void addBridge(ISBridge* bridge){
        mv_bridges.push_back(bridge);
    }
    virtual void on_received_data(SerializedPayload_t* payload) = 0;

    // Forbid copy
    ISSubscriber(const ISSubscriber&) = delete;
    ISSubscriber& operator=(const ISSubscriber&) = delete;
};

#endif // _Header__SUBSCRIBER_H_
