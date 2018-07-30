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

#include "FilePublisher.h"
#include "../../src/log/ISLog.h"
#include "HelloWorldPubSubTypes.h"

#include <iostream>

static const std::string s_sFilename("filename");
static const std::string s_sFormat("format");
static const std::string s_sTxt("txt");
static const std::string s_sAppend("append");
static const std::string s_sTrue("true");
static const std::string s_sFalse("false");

FilePublisher::~FilePublisher()
{
    file.close();
}

FilePublisher::FilePublisher(const std::string &name)
    : ISPublisher(name)
    , outputFile("output.txt")
    , fileFormat(s_sTxt)
    , append(false)
{
    file.open(outputFile, std::ios::out);
}

FilePublisher::FilePublisher(const std::string &name, const std::vector<std::pair<std::string, std::string>> *config)
    : ISPublisher(name)
    , outputFile("output.txt")
    , fileFormat(s_sTxt)
    , append(false)
{
    for (auto pair : *config)
    {
        try
        {
            if (pair.first.compare(s_sFilename) == 0)
            {
                outputFile = pair.second;
            }
            else if (pair.first.compare(s_sFormat) == 0)
            {
                if (pair.second.compare(s_sTxt) == 0)
                {
                    fileFormat = s_sTxt;
                }
                else
                {
                    LOG_ERROR("Unknown format");
                }
            }
            else if (pair.first.compare(s_sAppend) == 0)
            {
                append = pair.second.compare(s_sTrue) == 0;
            }
        }
        catch (...)
        {
            return;
        }
    }

    if (append)
    {
        file.open(outputFile, std::ios::out | std::ios::app);
    }
    else
    {
        file.open(outputFile, std::ios::out);
    }
}

bool FilePublisher::publish(SerializedPayload_t* payload)
{
    HelloWorldPubSubType hello_pst;
    HelloWorld hello;
    hello_pst.deserialize(payload, &hello);

    file << hello.message() << " : " << hello.index() << std::endl;
    return true;
}
