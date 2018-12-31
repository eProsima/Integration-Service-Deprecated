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

/**
 * @file TestSubscriber.cpp
 *
 */

#include "TestSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/utils/IPLocator.h>
#include <gtest/gtest.h>


#include <fastrtps/Domain.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TestSubscriber::TestSubscriber()
    : mp_participant(nullptr)
    , mp_subscriber(nullptr)
    , m_bInitialized(false)
	, m_subListener(this)
{
}

bool TestSubscriber::init(int transport, ReliabilityQosPolicyKind qosKind, const std::string& topicName,
        int domain, TopicDataType* type, const std::string& name, unsigned short port, const std::string& address)
{
    m_Name = name;
    m_Type = type;
    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = domain;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    PParam.rtps.setName(m_Name.c_str());

    if (transport == 1)
    {
        PParam.rtps.useBuiltinTransports = true;
    }
    else if (transport == 2)
    {
        int32_t kind = LOCATOR_KIND_TCPv4;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        IPLocator::setIPv4(initial_peer_locator, address);
        IPLocator::setPhysicalPort(initial_peer_locator, port);
        PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
        descriptor->wait_for_tcp_negotiation = false;
        descriptor->maxInitialPeersRange = 10;
        descriptor->logical_port_range = 100;
		descriptor->sendBufferSize = 8912896; // 8.5Mb
		descriptor->receiveBufferSize = 8912896; // 8.5Mb
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 3)
    {
        //uint32_t kind = LOCATOR_KIND_UDPv6;
        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<UDPv6TransportDescriptor> descriptor = std::make_shared<UDPv6TransportDescriptor>();
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 4)
    {
        uint32_t kind = LOCATOR_KIND_TCPv6;
        PParam.rtps.useBuiltinTransports = false;

        Locator_t initial_peer_locator;
        initial_peer_locator.kind = kind;
        IPLocator::setIPv6(initial_peer_locator, address);
        initial_peer_locator.port = port;
        PParam.rtps.builtin.initialPeersList.push_back(initial_peer_locator); // Publisher's meta channel

        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<TCPv6TransportDescriptor> descriptor = std::make_shared<TCPv6TransportDescriptor>();
		descriptor->sendBufferSize = 8912896; // 8.5Mb
		descriptor->receiveBufferSize = 8912896; // 8.5Mb
        PParam.rtps.userTransports.push_back(descriptor);
    }

    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant==nullptr)
        return false;

    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = type->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    Rparam.topic.topicDataType = type->getName();

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, type);

	Rparam.topic.topicName = topicName;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_reliability.kind = qosKind;
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_subListener);

    if (mp_subscriber == nullptr)
    {
        return false;
    }

    m_Data = m_Type->createData();

    m_bInitialized = true;

    return true;
}

TestSubscriber::~TestSubscriber()
{
    m_Type->deleteData(m_Data);
    Domain::removeParticipant(mp_participant);
}

TestSubscriber::SubListener::SubListener(TestSubscriber* parent)
    : mParent(parent)
    , n_matched(0)
    , n_samples(0)
{
}

void TestSubscriber::waitDiscovery(bool expectMatch, int maxWait)
{
    std::unique_lock<std::mutex> lock(m_mDiscovery);

    if(m_subListener.n_matched == 0)
        m_cvDiscovery.wait_for(lock, std::chrono::seconds(maxWait));

    if (expectMatch)
    {
        ASSERT_GE(m_subListener.n_matched, 1);
    }
    else
    {
        ASSERT_EQ(m_subListener.n_matched, 0);
    }
}

void TestSubscriber::matched(bool unmatched)
{
    std::unique_lock<std::mutex> lock(m_mDiscovery);
    if (unmatched)
    {
        --m_subListener.n_matched;
    }
    else
    {
        ++m_subListener.n_matched;
    }
    if(m_subListener.n_matched >= 1)
        m_cvDiscovery.notify_one();
}

void TestSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        mParent->matched();
        std::cout << mParent->m_Name << " matched."<<std::endl;
    }
    else
    {
        mParent->matched(true);
        std::cout << mParent->m_Name << " unmatched."<<std::endl;
    }
}

void TestSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    if (sub->takeNextData(mParent->m_Data, &m_info))
    {
        if (m_info.sampleKind == ALIVE)
        {
            ++n_samples;
            mParent->cv_.notify_one();
            //std::cout << mParent->m_Name << " received a total of " << n_samples << " samples." << std::endl;
        }
    }
}

void TestSubscriber::run()
{
    std::cout << m_Name << " running..." << std::endl;
	std::cin.ignore();
}
