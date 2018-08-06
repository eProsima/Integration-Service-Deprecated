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


#ifndef _FILE_PUBLISHER_H_
#define _FILE_PUBLISHER_H_

#include "../../src/ISBridge.h"
#include <iostream>
#include <fstream>

class FilePublisher : public ISPublisher
{
private:
    std::string url;
    std::string outputFile;
    std::string fileFormat;
    bool append;
    std::ofstream file;
public:
    FilePublisher(const std::string &name);
    FilePublisher(const std::string &name, const std::vector<std::pair<std::string, std::string>> *config);
    ~FilePublisher() override;

    bool publish(SerializedPayload_t*) override;
    bool publish(eprosima::fastrtps::types::DynamicData*) override { return false; };
};

#endif // _FILE_PUBLISHER_H_
