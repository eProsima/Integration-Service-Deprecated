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
}

void ISBridge::addFunction(const std::string &sub, const std::string &fname, userf_t func)
{
    mm_functionsNames[fname] = func;
    std::vector<std::string> &fnames = mm_functions[sub];
    fnames.push_back(fname);
    //mm_functions.emplace(sub, fname);
}

void ISBridge::addPublisher(const std::string &sub, const std::string &funcName, ISPublisher* pub)
{
    std::string key = generateKeyPublisher(sub, funcName);
    std::vector<ISPublisher*> &pubs = mm_publisher[key];
    pubs.emplace_back(pub);
    //mm_publisher.emplace(key, pub);
    mm_inv_publisher[pub] = key;
    ms_subpubs.emplace(pub);
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
}

void ISBridge::on_received_data(const ISSubscriber *sub, SerializedPayload_t *data)
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
