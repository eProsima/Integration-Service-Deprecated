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

#ifndef _ISBASECLASS_H_
#define _ISBASECLASS_H_

#include <string>
#include <fastrtps/TopicDataType.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class ISBaseClass
{
protected:
    std::string name;
    virtual void setName(const std::string &name) { this->name = name; }
public:

    ISBaseClass(const std::string &name) : name(name) {};
    virtual ~ISBaseClass() = default;

    virtual const std::string& getName() const { return name; }

    virtual void onTerminate() {};
};

#endif // _ISBASECLASS_H_
