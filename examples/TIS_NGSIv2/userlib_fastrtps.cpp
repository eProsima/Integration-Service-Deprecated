#include <iostream>
#include <sstream>
#include "RobotRcvPubSubTypes.h"
#include "JsonNGSIv2PubSubTypes.h"

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
	RobotRcv robot_data;
	RobotRcvPubSubType robot_pst;
    
    JsonNGSIv2PubSubType string_pst;
    JsonNGSIv2 string_data;

	// Deserialization
	robot_pst.deserialize(serialized_input, &robot_data);

	// Custom transformation
    std::stringstream ss;
    ss << "{ \"id\": \"" << robot_data.robot_id() << "\", \"type\": \"Robot\",";
    ss << "\"transmission_time\": { \"value\": \" " << robot_data.transmission_time() << "\", \"type\": \"DateTime\"}, ";
    ss << "\"floor\": {\"value\": " << robot_data.destination().floor() << "}, ";
    ss << "\"x\": {\"value\": " << robot_data.destination().x() << "}, ";
    ss << "\"y\": {\"value\": " << robot_data.destination().y() << "}, ";
    ss << "\"zeta\": {\"value\": " << robot_data.destination().zeta() << "},";
    ss << "\"state\": {\"value\": " << ((robot_data.state() == State::ACTION) ? "\"ACTION\"" : "\"STAND_BY\"") << "} }";
	string_data.data(ss.str());
    std::cout << string_data.data() << std::endl;
    
    
	// Serialization
	serialized_output->reserve(string_pst.m_typeSize);
	string_pst.serialize(&string_data, serialized_output);
}
