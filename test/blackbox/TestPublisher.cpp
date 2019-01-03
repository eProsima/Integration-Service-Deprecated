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
 * @file TestPublisher.cpp
 *
 */

#include "TestPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/utils/IPLocator.h>
#include <gtest/gtest.h>
#include <thread>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

TestPublisher::TestPublisher()
    : m_iSamples(-1)
    , m_sentSamples(0)
	, m_iWaitTime(1000)
    , m_bInitialized(false)
	, mp_participant(nullptr)
	, mp_publisher(nullptr)
	, m_pubListener(this)
{
}

bool TestPublisher::init(int transport, ReliabilityQosPolicyKind qosKind, int samples,
        int wait_time, const std::string& topicName, int domain, TopicDataType* type,
        const std::string& name, unsigned short port)
{
    m_Name = name;
    m_iSamples = samples;
	m_iWaitTime = wait_time;
    m_Type = type;

    ParticipantAttributes PParam;
    PParam.rtps.builtin.domainId = domain;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.builtin.leaseDuration_announcementperiod = Duration_t(1, 0);
    PParam.rtps.setName(m_Name.c_str());

    if (transport == 1)
    {
        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<UDPv4TransportDescriptor> descriptor = std::make_shared<UDPv4TransportDescriptor>();
        descriptor->maxInitialPeersRange = 20;
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 2)
    {
        //uint32_t kind = LOCATOR_KIND_TCPv4;
        PParam.rtps.useBuiltinTransports = false;

        std::shared_ptr<TCPv4TransportDescriptor> descriptor = std::make_shared<TCPv4TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->wait_for_tcp_negotiation = false;
        descriptor->keep_alive_frequency_ms = 1000;
        descriptor->keep_alive_timeout_ms = 5000;
        descriptor->add_listener_port(port);
        descriptor->maxInitialPeersRange = 20;
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 3)
    {
        //uint32_t kind = LOCATOR_KIND_UDPv6;
        PParam.rtps.useBuiltinTransports = false;
        std::shared_ptr<UDPv6TransportDescriptor> descriptor = std::make_shared<UDPv6TransportDescriptor>();
        descriptor->maxInitialPeersRange = 20;
        PParam.rtps.userTransports.push_back(descriptor);
    }
    else if (transport == 4)
    {
        uint32_t kind = LOCATOR_KIND_TCPv6;
        PParam.rtps.useBuiltinTransports = false;

        Locator_t unicast_locator;
        unicast_locator.kind = kind;
        IPLocator::setIPv6(unicast_locator, "::1");
        unicast_locator.port = port;
        PParam.rtps.defaultUnicastLocatorList.push_back(unicast_locator); // Publisher's data channel

        Locator_t meta_locator;
        meta_locator.kind = kind;
        IPLocator::setIPv6(meta_locator, "::1");
        meta_locator.port = port;
        PParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(meta_locator);  // Publisher's meta channel

        std::shared_ptr<TCPv6TransportDescriptor> descriptor = std::make_shared<TCPv6TransportDescriptor>();
        descriptor->sendBufferSize = 8912896; // 8.5Mb
        descriptor->receiveBufferSize = 8912896; // 8.5Mb
        descriptor->wait_for_tcp_negotiation = false;
        descriptor->keep_alive_frequency_ms = 1000;
        descriptor->keep_alive_timeout_ms = 5000;
        descriptor->add_listener_port(port);
        descriptor->maxInitialPeersRange = 20;
        PParam.rtps.userTransports.push_back(descriptor);
    }

    mp_participant = Domain::createParticipant(PParam);

    if (mp_participant == nullptr)
    {
        return false;
    }

	// CREATE THE PUBLISHER
	PublisherAttributes Wparam;
	Wparam.topic.topicKind = type->m_isGetKeyDefined ? WITH_KEY : NO_KEY;
    Wparam.topic.topicDataType = type->getName();

    //REGISTER THE TYPE
    Domain::registerType(mp_participant, type);

	Wparam.topic.topicName = topicName;
    Wparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Wparam.qos.m_reliability.kind = qosKind;
    Wparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_pubListener);
    if (mp_publisher == nullptr)
    {
        return false;
    }

    m_Data = m_Type->createData();

    m_bInitialized = true;

    return true;
}

TestPublisher::~TestPublisher()
{
    m_Type->deleteData(m_Data);
    Domain::removeParticipant(mp_participant);
}

void TestPublisher::waitDiscovery(bool expectMatch, int maxWait)
{
    std::unique_lock<std::mutex> lock(m_mDiscovery);

    if(m_pubListener.n_matched == 0)
        m_cvDiscovery.wait_for(lock, std::chrono::seconds(maxWait));

    if (expectMatch)
    {
        ASSERT_GE(m_pubListener.n_matched, 1);
    }
    else
    {
        ASSERT_EQ(m_pubListener.n_matched, 0);
    }
}

void TestPublisher::matched()
{
    std::unique_lock<std::mutex> lock(m_mDiscovery);
    ++m_pubListener.n_matched;
    if(m_pubListener.n_matched >= 1)
        m_cvDiscovery.notify_one();
}

TestPublisher::PubListener::PubListener(TestPublisher* parent)
    : mParent(parent)
	, n_matched(0)
{
}

void TestPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        std::cout << mParent->m_Name << " matched." << std::endl;
        mParent->matched();
    }
    else
    {
        std::cout << mParent->m_Name << " unmatched." << std::endl;
    }
}

void TestPublisher::runThread()
{
	int iPrevCount = 0;
	std::cout << m_Name << " running..." << std::endl;
    while (!publish() && iPrevCount < m_iSamples)
    {
        eClock::my_sleep(m_iWaitTime);
        ++iPrevCount;
    }
}

void TestPublisher::run()
{
    std::thread thread(&TestPublisher::runThread, this);
    thread.join();
}

bool TestPublisher::publish()
{
    if (m_pubListener.n_matched > 0)
    {
        if (mp_publisher->write(m_Data))
        {
            ++m_sentSamples;
            //std::cout << m_Name << " sent a total of " << m_sentSamples << " samples." << std::endl;
            return true;
        }
        //else
        //{
        //    std::cout << m_Name << " failed to send " << (m_sentSamples + 1) << " sample." << std::endl;
        //}
    }
    return false;
}
