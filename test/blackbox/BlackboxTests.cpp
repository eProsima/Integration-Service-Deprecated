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

TEST_F(BlackBox, String_1_1_UDP_UDP_Static)
{
    StringType type;
    TestPublisher pub;
    TestSubscriber sub;
    TestIS is("config/String_1_1_UDP_UDP_Static.xml");
    const uint16_t nmsgs = 5;

    pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub_String");

    ASSERT_TRUE(pub.isInitialized());

    sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub_String");

    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery. They must not discover each other.
    pub.waitDiscovery(false, 1);
    sub.waitDiscovery(false, 1);

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub.waitDiscovery();
    pub.waitDiscovery();

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub.send();
        //pub.block(std::chrono::seconds(10));
        eClock::my_sleep(5);
    }

    //eClock::my_sleep(500);
    sub.block_for_at_least(3);

    ASSERT_GE(sub.samplesReceived(), 3);
    is.stop();
}

TEST_F(BlackBox, String_1_5_UDP_UDP_Static)
{
    StringType type;
    TestPublisher pub;
    TestSubscriber sub1;
    TestSubscriber sub2;
    TestSubscriber sub3;
    TestSubscriber sub4;
    TestSubscriber sub5;
    TestIS is("config/String_1_5_UDP_UDP_Static.xml");
    const uint16_t nmsgs = 5;

    pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub_String");

    ASSERT_TRUE(pub.isInitialized());

    sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub1_String");
    sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 6, &type, "Sub2_String");
    sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 7, &type, "Sub3_String");
    sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 8, &type, "Sub4_String");
    sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 9, &type, "Sub5_String");

    ASSERT_TRUE(sub1.isInitialized());
    ASSERT_TRUE(sub2.isInitialized());
    ASSERT_TRUE(sub3.isInitialized());
    ASSERT_TRUE(sub4.isInitialized());
    ASSERT_TRUE(sub5.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub1.waitDiscovery();
    sub2.waitDiscovery();
    sub3.waitDiscovery();
    sub4.waitDiscovery();
    sub5.waitDiscovery();
    pub.waitDiscovery();

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub.send();
        //pub.block(std::chrono::seconds(10));
        eClock::my_sleep(5);
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

TEST_F(BlackBox, String_3_1_UDP_UDP_Static)
{
    StringType type;
    TestPublisher pub1;
    TestPublisher pub2;
    TestPublisher pub3;
    TestSubscriber sub;
    TestIS is("config/String_3_1_UDP_UDP_Static.xml");
    const uint16_t nmsgs = 5;

    pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
    pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 1, &type, "Pub2_String");
    pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 2, &type, "Pub3_String");

    ASSERT_TRUE(pub1.isInitialized());
    ASSERT_TRUE(pub2.isInitialized());
    ASSERT_TRUE(pub3.isInitialized());

    sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub_String");

    ASSERT_TRUE(sub.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub.waitDiscovery();
    pub1.waitDiscovery();
    pub2.waitDiscovery();
    pub3.waitDiscovery();

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub1.send();
        eClock::my_sleep(0);
        pub2.send();
        eClock::my_sleep(0);
        pub3.send();
        eClock::my_sleep(0);
    }

    //eClock::my_sleep(500);
    sub.block_for_at_least(9);

    ASSERT_GE(sub.samplesReceived(), 3);
    is.stop();
}

TEST_F(BlackBox, String_3_5_UDP_UDP_Static)
{
    StringType type;
    TestPublisher pub1;
    TestPublisher pub2;
    TestPublisher pub3;
    TestSubscriber sub1;
    TestSubscriber sub2;
    TestSubscriber sub3;
    TestSubscriber sub4;
    TestSubscriber sub5;
    TestIS is("config/String_3_5_UDP_UDP_Static.xml");
    const uint16_t nmsgs = 5;

    pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
    pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 1, &type, "Pub2_String");
    pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 2, &type, "Pub3_String");

    ASSERT_TRUE(pub1.isInitialized());
    ASSERT_TRUE(pub2.isInitialized());
    ASSERT_TRUE(pub3.isInitialized());

    sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub1_String");
    sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 6, &type, "Sub2_String");
    sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 7, &type, "Sub3_String");
    sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 8, &type, "Sub4_String");
    sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 9, &type, "Sub5_String");

    ASSERT_TRUE(sub1.isInitialized());
    ASSERT_TRUE(sub2.isInitialized());
    ASSERT_TRUE(sub3.isInitialized());
    ASSERT_TRUE(sub4.isInitialized());
    ASSERT_TRUE(sub5.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub1.waitDiscovery();
    sub2.waitDiscovery();
    sub3.waitDiscovery();
    sub4.waitDiscovery();
    sub5.waitDiscovery();
    pub1.waitDiscovery();
    pub2.waitDiscovery();
    pub3.waitDiscovery();

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub1.send();
        eClock::my_sleep(0);
        pub2.send();
        eClock::my_sleep(0);
        pub3.send();
        eClock::my_sleep(0);
    }

    //eClock::my_sleep(500);
    sub1.block_for_at_least(9);
    sub2.block_for_at_least(9);
    sub3.block_for_at_least(9);
    sub4.block_for_at_least(9);
    sub5.block_for_at_least(9);

    ASSERT_GE(sub1.samplesReceived(), 9);
    ASSERT_GE(sub2.samplesReceived(), 9);
    ASSERT_GE(sub3.samplesReceived(), 9);
    ASSERT_GE(sub4.samplesReceived(), 9);
    ASSERT_GE(sub5.samplesReceived(), 9);
    is.stop();
}

/*
TEST_F(BlackBox, Data1mb_1_1_UDP_UDP_Static)
{
    Data1mbType type;
    TestPublisher pub;
    TestSubscriber sub;
    TestIS is("config/Data1mb_1_1_UDP_UDP_Static.xml");

    const uint16_t nmsgs = 5;

    sub.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 5, &type, "Sub_Data1mb");

    ASSERT_TRUE(sub.isInitialized());

    pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb");

    ASSERT_TRUE(pub.isInitialized());

    is.run();

    std::cout << "IS Running" << std::endl;

    // Wait for discovery.
    sub.waitDiscovery();
    pub.waitDiscovery();

    std::cout << "Discovery." << std::endl;

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub.send();
        //pub.block(std::chrono::seconds(10));
        eClock::my_sleep(5);
    }

    //eClock::my_sleep(500);
    sub.block_for_at_least(5);

    ASSERT_GE(sub.samplesReceived(), 5);
    is.stop();
}
*/

TEST_F(BlackBox, Data1mb_1_5_UDP_UDP_Static)
{
    Data1mbType type;
    TestPublisher pub;
    TestSubscriber sub1;
    TestSubscriber sub2;
    TestSubscriber sub3;
    TestSubscriber sub4;
    TestSubscriber sub5;
    TestIS is("config/Data1mb_1_5_UDP_UDP_Static.xml");
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

    pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb");

    ASSERT_TRUE(pub.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub1.waitDiscovery();
    sub2.waitDiscovery();
    sub3.waitDiscovery();
    sub4.waitDiscovery();
    sub5.waitDiscovery();
    pub.waitDiscovery();

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub.send();
        //pub.block(std::chrono::seconds(10));
        eClock::my_sleep(5);
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

TEST_F(BlackBox, String_3_1_UDP_UDP_Dynamic)
{
    StringType type;
    TestPublisher pub1;
    TestPublisher pub2;
    TestPublisher pub3;
    TestSubscriber sub;
    TestIS is("config/String_3_1_UDP_UDP_Dynamic.xml");
    const uint16_t nmsgs = 5;

    pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
    pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 1, &type, "Pub2_String");
    pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 2, &type, "Pub3_String");

    ASSERT_TRUE(pub1.isInitialized());
    ASSERT_TRUE(pub2.isInitialized());
    ASSERT_TRUE(pub3.isInitialized());

    sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub_String");

    ASSERT_TRUE(sub.isInitialized());

    // Launch IS, they will discover each other
    is.run();

    // Wait for discovery.
    sub.waitDiscovery();
    pub1.waitDiscovery();
    pub2.waitDiscovery();
    pub3.waitDiscovery();

    for(uint16_t count = 0; count < nmsgs; ++count)
    {
        pub1.send();
        eClock::my_sleep(0);
        pub2.send();
        eClock::my_sleep(0);
        pub3.send();
        eClock::my_sleep(0);
    }

    //eClock::my_sleep(500);
    sub.block_for_at_least(9);

    ASSERT_GE(sub.samplesReceived(), 3);
    is.stop();
}

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

    pub1.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "KeyedTopic", 0, &type, "Pub1_Keyed", 6200);
    pub2.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "KeyedTopic", 1, &type, "Pub2_Keyed", 6201);
    pub3.init(TCP, RELIABLE_RELIABILITY_QOS, 5, 10, "KeyedTopic", 2, &type, "Pub3_Keyed", 6202);

    ASSERT_TRUE(pub1.isInitialized());
    ASSERT_TRUE(pub2.isInitialized());
    ASSERT_TRUE(pub3.isInitialized());

    sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 5, &type, "Sub1_Keyed");
    sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 6, &type, "Sub2_Keyed");
    sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 7, &type, "Sub3_Keyed");
    sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 8, &type, "Sub4_Keyed");
    sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "KeyedTopic", 9, &type, "Sub5_Keyed");

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
