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

#ifndef _GENERIC_PUBSUBTYPES_H_
#define _GENERIC_PUBSUBTYPES_H_

#include <fastrtps/TopicDataType.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class GenericPubSubType : public TopicDataType {
public:
	GenericPubSubType();
	virtual ~GenericPubSubType();
	bool serialize(void *data, SerializedPayload_t *payload);
	bool deserialize(SerializedPayload_t *payload, void *data);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
	void* createData() { return 0; }
	void deleteData(void*){ }
	MD5 m_md5;
	unsigned char* m_keyBuffer;
};

#endif // _GENERIC_PUBSUBTYPE_H_
