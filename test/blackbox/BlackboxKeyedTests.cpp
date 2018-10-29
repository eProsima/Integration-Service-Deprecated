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

#include "types/KeyedPubSubTypes.h"

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

TEST_F(BlackBox, Keyed_3_5_TCP_UDP_Dynamic)
{
    KeyedPubSubType type;
    TestPublisher pub1;
    TestPublisher pub2;
    TestPublisher pub3;
    TestSubscriber sub1;
    TestSubscriber sub2;
    TestSubscriber sub3;
    TestSubscriber sub4;
    TestSubscriber sub5;
    TestIS is("config/Keyed_3_5_TCP_UDP_Dynamic.xml");
    const uint16_t nmsgs = 5;

    pub1.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "KeyedTopic", 2912, &type, "Pub1_Keyed", 6200);
    pub2.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "KeyedTopic", 7318, &type, "Pub2_Keyed", 6201);
    pub3.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "KeyedTopic", 2603, &type, "Pub3_Keyed", 6202);

    ASSERT_TRUE(pub1.isInitialized());
    ASSERT_TRUE(pub2.isInitialized());
    ASSERT_TRUE(pub3.isInitialized());

    sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 6374, &type, "Sub1_Keyed");
    sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 8449, &type, "Sub2_Keyed");
    sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 8133, &type, "Sub3_Keyed");
    sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 5076, &type, "Sub4_Keyed");
    sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 3886, &type, "Sub5_Keyed");

    ASSERT_TRUE(sub1.isInitialized());
    ASSERT_TRUE(sub2.isInitialized());
    ASSERT_TRUE(sub3.isInitialized());
    ASSERT_TRUE(sub4.isInitialized());
    ASSERT_TRUE(sub5.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub1.waitDiscovery(true, 30);
    sub2.waitDiscovery(true, 30);
    sub3.waitDiscovery(true, 30);
    sub4.waitDiscovery(true, 30);
    sub5.waitDiscovery(true, 30);
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
    sub1.block_for_at_least(3);
    sub2.block_for_at_least(3);
    sub3.block_for_at_least(3);
    sub4.block_for_at_least(3);
    sub5.block_for_at_least(3);

    ASSERT_GE(sub1.samplesReceived(), 3);
    ASSERT_GE(sub2.samplesReceived(), 3);
    ASSERT_GE(sub3.samplesReceived(), 3);
    ASSERT_GE(sub4.samplesReceived(), 3);
    ASSERT_GE(sub5.samplesReceived(), 3);
    is.stop();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
