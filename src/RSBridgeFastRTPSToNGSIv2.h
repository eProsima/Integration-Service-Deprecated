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


#ifndef _RSBRIDGEFASTRTPSTONGSIv2_H_
#define _RSBRIDGEFASTRTPSTONGSIv2_H_

#include <iostream>

#include <fastrtps/fastrtps_fwd.h>
#include "fastrtps/participant/Participant.h"
#include "fastrtps/attributes/ParticipantAttributes.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/attributes/PublisherAttributes.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/attributes/SubscriberAttributes.h"
#include "GenericPubSubTypes.h"
#include "NGSIv2Params.h"

#include "dynamicload/dynamicload.h"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <boost/asio.hpp>
#include <boost/array.hpp>

using namespace std;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace curlpp::options;
using namespace curlpp::infos;

typedef void (*userf_t)(SerializedPayload_t *serialized_input, SerializedPayload_t *serialized_output);

class RSBridgeFastRTPSToNGSIv2
{
public:
    RSBridgeFastRTPSToNGSIv2(
        ParticipantAttributes par_fastrtps_params,
        NGSIv2Params par_ngsiv2_params,
        SubscriberAttributes sub_params,
        const char* file_path);
    virtual ~RSBridgeFastRTPSToNGSIv2();
private:
    Participant *mf_participant;
    Subscriber *mf_subscriber;
    GenericPubSubType *data_type;
    std::string ngsiv2_host;
    uint16_t ngsiv2_port;
    std::string ngsiv2_id;
    
    class NGSIv2Publisher
    {
    public:
        string url;
        NGSIv2Publisher();
        NGSIv2Publisher(const string host, const uint16_t port);
        void setHostPort(const string host, const uint16_t port);
        ~NGSIv2Publisher();
        string write(SerializedPayload_t *payload);
    } ngsiv2_publisher;

    class SubListener : public SubscriberListener{
    public:
        SubListener();
        SubListener(const char* file_path);
        ~SubListener();
        void loadLibrary(const char* file_path);
        void setPublisher(NGSIv2Publisher* publisher){
            listener_publisher = publisher;
        }
        void onSubscriptionMatched(Subscriber* sub, MatchingInfo& info);
        void onNewDataMessage(Subscriber* sub);
        NGSIv2Publisher *listener_publisher;
        userf_t user_transformation;
        void *handle;
        char *error;

        SampleInfo_t m_info;
        int n_matched;
        int n_msg;
    } m_listener;
};

#endif // _Header__SUBSCRIBER_H_

