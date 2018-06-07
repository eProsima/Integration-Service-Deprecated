// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*! 
 * @file ShapePubSubTypes.cpp
 * This header file contains the implementation of the serialization functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <ShapePubSubTypes.h>
#include <iostream>

ShapeTypePubSubType::ShapeTypePubSubType() {
    setName("ShapeType");
    m_typeSize = (uint32_t)ShapeType::getMaxCdrSerializedSize() + 4 /*encapsulation*/;
    m_isGetKeyDefined = ShapeType::isKeyDefined();
    m_keyBuffer = (unsigned char*)malloc(ShapeType::getKeyMaxCdrSerializedSize()>16 ? ShapeType::getKeyMaxCdrSerializedSize() : 16);
}

ShapeTypePubSubType::~ShapeTypePubSubType() {
    if(m_keyBuffer!=nullptr)
        free(m_keyBuffer);
}

bool ShapeTypePubSubType::serialize(void *data, rtps::SerializedPayload_t *payload) {
    ShapeType *p_type = (ShapeType*) data;
    eprosima::fastcdr::FastBuffer fastbuffer((char*) payload->data, payload->max_size); // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    // Serialize encapsulation
    ser.serialize_encapsulation();

    try
    {
        p_type->serialize(ser); 	// Serialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    payload->length = (uint32_t)ser.getSerializedDataLength(); 	//Get the serialized length
    return true;
}

bool ShapeTypePubSubType::deserialize(rtps::SerializedPayload_t* payload, void* data) {
    ShapeType* p_type = (ShapeType*) data; 	//Convert DATA to pointer of your type
    eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length); 	// Object that manages the raw buffer.
    eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR); // Object that deserializes the data.
    // Deserialize encapsulation.
    deser.read_encapsulation();
    payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

    try
    {
        p_type->deserialize(deser);	//Deserialize the object:
        //std::cout << "Deserialized: " << p_type->color() << std::endl;
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }

    return true;
}

std::function<uint32_t()> ShapeTypePubSubType::getSerializedSizeProvider(void* data) {
    return [data]() -> uint32_t { return (uint32_t)type::getCdrSerializedSize(*static_cast<ShapeType*>(data)); };
}

void* ShapeTypePubSubType::createData() {
    return (void*)new ShapeType();
}

void ShapeTypePubSubType::deleteData(void* data) {
    delete((ShapeType*)data);
}

bool ShapeTypePubSubType::getKey(void *data, rtps::InstanceHandle_t* handle) {
    if(!m_isGetKeyDefined)
        return false;
    ShapeType* p_type = (ShapeType*) data;
    eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer,ShapeType::getKeyMaxCdrSerializedSize()); 	// Object that manages the raw buffer.
    eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS); 	// Object that serializes the data.
    p_type->serializeKey(ser);
    if(ShapeType::getKeyMaxCdrSerializedSize()>16)	{
        m_md5.init();
        m_md5.update(m_keyBuffer,(unsigned int)ser.getSerializedDataLength());
        m_md5.finalize();
        for(uint8_t i = 0;i<16;++i)    	{
            handle->value[i] = m_md5.digest[i];
        }
    }
    else    {
        for(uint8_t i = 0;i<16;++i)    	{
            handle->value[i] = m_keyBuffer[i];
        }
    }
    return true;
}

