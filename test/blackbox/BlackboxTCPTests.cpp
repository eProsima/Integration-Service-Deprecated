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

#include "types/StringType.h"
#include "types/Data1mbType.h"

#include <thread>

#include "TestPublisher.h"
#include "TestSubscriber.h"
#include "TestIS.h"

#include <fastrtps/Domain.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>
#include <fastrtps/utils/eClock.h>

#include <thread>
#include <memory>
#include <cstdlib>
#include <string>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum Transports
{
    UDP = 1,
    TCP,
    UDP6,
    TCP6
};

class BlackBox : public ::testing::Test
{
    public:

        BlackBox()
        {
            //Log::SetVerbosity(Log::Info);
            //Log::SetCategoryFilter(std::regex("(SECURITY)"));
        }

        ~BlackBox()
        {
            //Log::Reset();
            Log::KillThread();
            eprosima::fastrtps::Domain::stopAll();
        }
};

TEST_F(BlackBox, Data1mb_1_5_TCP_UDP_Static)
{
    Data1mbType type;
    TestPublisher pub;
    TestSubscriber sub1;
    TestSubscriber sub2;
    TestSubscriber sub3;
    TestSubscriber sub4;
    TestSubscriber sub5;
    TestIS is("config/Data1mb_1_5_TCP_UDP_Static.xml");
    const uint16_t nmsgs = 5;

    sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 5, &type, "Sub1_Data1mb");
    sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 6, &type, "Sub2_Data1mb");
    sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 7, &type, "Sub3_Data1mb");
    sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 8, &type, "Sub4_Data1mb");
    sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 9, &type, "Sub5_Data1mb");

    ASSERT_TRUE(sub1.isInitialized());
    ASSERT_TRUE(sub2.isInitialized());
    ASSERT_TRUE(sub3.isInitialized());
    ASSERT_TRUE(sub4.isInitialized());
    ASSERT_TRUE(sub5.isInitialized());

    pub.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb", 5100);

    ASSERT_TRUE(pub.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub1.waitDiscovery(true, 30);
    sub2.waitDiscovery(true, 30);
    sub3.waitDiscovery(true, 30);
    sub4.waitDiscovery(true, 30);
    sub5.waitDiscovery(true, 30);
    pub.waitDiscovery(true, 30);

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub.send();
        //pub.block(std::chrono::seconds(10));
        eClock::my_sleep(50);
    }

    //eClock::my_sleep(500);
    sub1.block_for_at_least(1);
    sub2.block_for_at_least(1);
    sub3.block_for_at_least(1);
    sub4.block_for_at_least(1);
    sub5.block_for_at_least(1);

    ASSERT_GE(sub1.samplesReceived(), 1);
    ASSERT_GE(sub2.samplesReceived(), 1);
    ASSERT_GE(sub3.samplesReceived(), 1);
    ASSERT_GE(sub4.samplesReceived(), 1);
    ASSERT_GE(sub5.samplesReceived(), 1);
    is.stop();
}

TEST_F(BlackBox, Data1mb_3_1_UDP_TCP_Static)
{
    Data1mbType type;
    TestPublisher pub1;
    TestPublisher pub2;
    TestPublisher pub3;
    TestSubscriber sub;
    TestIS is("config/Data1mb_3_1_UDP_TCP_Static.xml");
    const uint16_t nmsgs = 5;

    pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub1_Data1mb");
    pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 1, &type, "Pub2_Data1mb");
    pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 2, &type, "Pub3_Data1mb");

    ASSERT_TRUE(pub1.isInitialized());
    ASSERT_TRUE(pub2.isInitialized());
    ASSERT_TRUE(pub3.isInitialized());

    sub.init(TCP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 5, &type, "Pub_Data1mb", 5101);

    ASSERT_TRUE(sub.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub.waitDiscovery(true, 30);
    pub1.waitDiscovery(true, 30);
    pub2.waitDiscovery(true, 30);
    pub3.waitDiscovery(true, 30);

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub1.send();
        eClock::my_sleep(10);
        pub2.send();
        eClock::my_sleep(10);
        pub3.send();
        eClock::my_sleep(10);
    }

    //eClock::my_sleep(500);
    sub.block_for_at_least(3);

    ASSERT_GE(sub.samplesReceived(), 3);
    is.stop();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
