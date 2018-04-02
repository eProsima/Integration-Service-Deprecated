#include <iostream>
#include "RobotSndPubSubTypes.h"
#include "JsonNGSIv2PubSubTypes.h"

#include "json/json.h"

#if defined(_WIN32) && defined (BUILD_SHARED_LIBS)
#if defined (_MSC_VER)
#pragma warning(disable: 4251)
#endif
#if defined(userlib_EXPORTS)
#define  USER_LIB_EXPORT __declspec(dllexport)
#else
#define  USER_LIB_EXPORT __declspec(dllimport)
#endif
#else
#define USER_LIB_EXPORT
#endif

using namespace eprosima::fastrtps::rtps;

extern "C" void USER_LIB_EXPORT transform(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output){
	// User types
	RobotSnd robot_data;
	RobotSndPubSubType robot_pst;
    
    JsonNGSIv2PubSubType string_pst;
    JsonNGSIv2 string_data;

	// Deserialization
	string_pst.deserialize(serialized_input, &string_data);

    /*
     
    ss << "{ \"id\": \"" << robot_data.robot_id() << "\", \"type\": \"Robot\",";
    ss << "\"transmission_time\": { \"value\": \" " << robot_data.transmission_time() << "\", \"type\": \"DateTime\""}, ";
    ss << "\"floor\": {\"value\": " << robot_data.position.floor() << "}, ";
    ss << "\"x\": {\"value\": " << robot_data.position.x() << "}, ";
    ss << "\"y\": {\"value\": " << robot_data.position.y() << "}, ";
    ss << "\"zeta\": {\"value\": " << robot_data.position.zeta() << "} }"; 
     
     */
    
	// Custom transformation
    Json::Value root;
    std::stringstream ss(string_data.data());
    ss >> root;
    
    robot_data.robot_id(root["id"].asString());
    robot_data.transmission_time(root["transmission_time"]["value"].asString());
    robot_data.position().floor(root["floor"]["value"].asLargestUInt());
    robot_data.position().x(root["x"]["value"].asLargestUInt());
    robot_data.position().y(root["y"]["value"].asLargestUInt());
    robot_data.position().zeta(root["zeta"]["value"].asLargestUInt());

	// Serialization
	serialized_output->reserve(robot_pst.m_typeSize);
	robot_pst.serialize(&robot_data, serialized_output);
}
