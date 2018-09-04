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
#include <chrono>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum Transports
{
    UDP = 1,
    TCP,
    UDP6,
    TCP6
};

class Benchmark : public ::testing::Test
{
    public:

        Benchmark()
        {
            //Log::SetVerbosity(Log::Info);
            //Log::SetCategoryFilter(std::regex("(SECURITY)"));
        }

        ~Benchmark()
        {
            //Log::Reset();
            Log::KillThread();
            eprosima::fastrtps::Domain::stopAll();
        }
};

class Chrono
{
public:
    Chrono(double& result) : m_Result(result)
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    ~Chrono()
    {
        m_Result = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - m_Start).count();
    }
private:
    double& m_Result;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

TEST_F(Benchmark, String_1_1_Static)
{
    StringType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub;
        TestSubscriber sub;

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub_String");

        ASSERT_TRUE(pub.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub_String");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub.waitDiscovery();
        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs, nmsgs);
        }

        if (recv < nmsgs)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostD = 100. - (recv * 100.) / nmsgs;
        }
    }

    // IS communication
    {
        TestPublisher pub;
        TestSubscriber sub;
        TestIS is("config/String_1_1_Static.xml");

        // Launch IS, they will discover each other
        is.run();

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub_String");

        ASSERT_TRUE(pub.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub_String");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub.waitDiscovery();
        eClock::my_sleep(500); // Wait a moment to match correctly

        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs, nmsgs);
        }

        if (recv < nmsgs)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostI = 100. - (recv * 100.) / nmsgs;
        }

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, String_1_5_Static)
{
    StringType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub_String");

        ASSERT_TRUE(pub.isInitialized());

        sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub1_String");
        sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub2_String");
        sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub3_String");
        sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub4_String");
        sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub5_String");

        ASSERT_TRUE(sub1.isInitialized());
        ASSERT_TRUE(sub2.isInitialized());
        ASSERT_TRUE(sub3.isInitialized());
        ASSERT_TRUE(sub4.isInitialized());
        ASSERT_TRUE(sub5.isInitialized());

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub.waitDiscovery();
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;

        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs, nmsgs / 16);
        }
        if (recv1 < nmsgs)
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostD += 100. - (recv1 * 100.) / nmsgs;
        }
        if (recv2 < nmsgs)
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostD += 100. - (recv2 * 100.) / nmsgs;
        }
        if (recv3 < nmsgs)
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostD += 100. - (recv3 * 100.) / nmsgs;
        }
        if (recv4 < nmsgs)
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostD += 100. - (recv4 * 100.) / nmsgs;
        }
        if (recv5 < nmsgs)
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostD += 100. - (recv5 * 100.) / nmsgs;
        }
        lostD /= 5.;

    }

    // IS communication
    {
        TestPublisher pub;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;
        TestIS is("config/String_1_5_Static.xml");

        // Launch IS, they will discover each other
        is.run();

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

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub.waitDiscovery();
        eClock::my_sleep(10000); // Wait a moment to match correctly
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;

        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs, nmsgs / 16);
        }
        if (recv1 < nmsgs)
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostI += 100. - (recv1 * 100.) / nmsgs;
        }
        if (recv2 < nmsgs)
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostI += 100. - (recv2 * 100.) / nmsgs;
        }
        if (recv3 < nmsgs)
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostI += 100. - (recv3 * 100.) / nmsgs;
        }
        if (recv4 < nmsgs)
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostI += 100. - (recv4 * 100.) / nmsgs;
        }
        if (recv5 < nmsgs)
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostI += 100. - (recv5 * 100.) / nmsgs;
        }
        lostI /= 5.;

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, String_3_1_Static)
{
    StringType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub;

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub2_String");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub3_String");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub_String");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs*3, nmsgs);
        }

        if (recv < nmsgs*3)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostD = 100. - (recv * 100.) / (nmsgs*3);
        }

    }

    // IS communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub;
        TestIS is("config/String_3_1_Static.xml");

        // Launch IS, they will discover each other
        is.run();

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 1, &type, "Pub2_String");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 2, &type, "Pub3_String");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 5, &type, "Sub_String");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        eClock::my_sleep(500); // Wait a moment to match correctly

        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs*3, nmsgs);
        }

        if (recv < nmsgs*3)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostI = 100. - (recv * 100.) / (nmsgs*3);
        }

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, String_3_5_Static)
{
    StringType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub2_String");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub3_String");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub1_String");
        sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub2_String");
        sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub3_String");
        sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub4_String");
        sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub5_String");

        ASSERT_TRUE(sub1.isInitialized());
        ASSERT_TRUE(sub2.isInitialized());
        ASSERT_TRUE(sub3.isInitialized());
        ASSERT_TRUE(sub4.isInitialized());
        ASSERT_TRUE(sub5.isInitialized());

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs*3, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs*3, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs*3, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs*3, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs*3, nmsgs / 16);
        }
        if (recv1 < (nmsgs*3))
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostD += 100. - (recv1 * 100.) / (nmsgs*3);
        }
        if (recv2 < (nmsgs*3))
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostD += 100. - (recv2 * 100.) / (nmsgs*3);
        }
        if (recv3 < (nmsgs*3))
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostD += 100. - (recv3 * 100.) / (nmsgs*3);
        }
        if (recv4 < (nmsgs*3))
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostD += 100. - (recv4 * 100.) / (nmsgs*3);
        }
        if (recv5 < (nmsgs*3))
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostD += 100. - (recv5 * 100.) / (nmsgs*3);
        }
        lostD /= 5.;

    }

    // IS communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;
        TestIS is("config/String_3_5_Static.xml");

        // Launch IS, they will discover each other
        is.run();

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

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        eClock::my_sleep(500); // Wait a moment to match correctly

        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs*3, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs*3, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs*3, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs*3, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs*3, nmsgs / 16);
        }
        if (recv1 < (nmsgs*3))
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostI += 100. - (recv1 * 100.) / (nmsgs*3);
        }
        if (recv2 < (nmsgs*3))
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostI += 100. - (recv2 * 100.) / (nmsgs*3);
        }
        if (recv3 < (nmsgs*3))
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostI += 100. - (recv3 * 100.) / (nmsgs*3);
        }
        if (recv4 < (nmsgs*3))
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostI += 100. - (recv4 * 100.) / (nmsgs*3);
        }
        if (recv5 < (nmsgs*3))
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostI += 100. - (recv5 * 100.) / (nmsgs*3);
        }
        lostI /= 5.;

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, String_3_5_Dynamic)
{
    StringType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub1_String");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub2_String");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "StringType", 0, &type, "Pub3_String");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub1_String");
        sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub2_String");
        sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub3_String");
        sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub4_String");
        sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "StringType", 0, &type, "Sub5_String");

        ASSERT_TRUE(sub1.isInitialized());
        ASSERT_TRUE(sub2.isInitialized());
        ASSERT_TRUE(sub3.isInitialized());
        ASSERT_TRUE(sub4.isInitialized());
        ASSERT_TRUE(sub5.isInitialized());

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs*3, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs*3, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs*3, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs*3, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs*3, nmsgs / 16);
        }
        if (recv1 < (nmsgs*3))
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostD += 100. - (recv1 * 100.) / (nmsgs*3);
        }
        if (recv2 < (nmsgs*3))
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostD += 100. - (recv2 * 100.) / (nmsgs*3);
        }
        if (recv3 < (nmsgs*3))
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostD += 100. - (recv3 * 100.) / (nmsgs*3);
        }
        if (recv4 < (nmsgs*3))
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostD += 100. - (recv4 * 100.) / (nmsgs*3);
        }
        if (recv5 < (nmsgs*3))
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostD += 100. - (recv5 * 100.) / (nmsgs*3);
        }
        lostD /= 5.;

    }

    // IS communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;
        TestIS is("config/String_3_5_Dynamic.xml");

        // Launch IS, they will discover each other
        is.run();

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

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        eClock::my_sleep(500); // Wait a moment to match correctly

        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs*3, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs*3, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs*3, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs*3, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs*3, nmsgs / 16);
        }
        if (recv1 < (nmsgs*3))
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostI += 100. - (recv1 * 100.) / (nmsgs*3);
        }
        if (recv2 < (nmsgs*3))
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostI += 100. - (recv2 * 100.) / (nmsgs*3);
        }
        if (recv3 < (nmsgs*3))
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostI += 100. - (recv3 * 100.) / (nmsgs*3);
        }
        if (recv4 < (nmsgs*3))
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostI += 100. - (recv4 * 100.) / (nmsgs*3);
        }
        if (recv5 < (nmsgs*3))
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostI += 100. - (recv5 * 100.) / (nmsgs*3);
        }
        lostI /= 5.;

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, Data1mb_1_5_Static)
{
    Data1mbType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb");

        ASSERT_TRUE(pub.isInitialized());

        sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub1_Data1mb");
        sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub2_Data1mb");
        sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub3_Data1mb");
        sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub4_Data1mb");
        sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub5_Data1mb");

        ASSERT_TRUE(sub1.isInitialized());
        ASSERT_TRUE(sub2.isInitialized());
        ASSERT_TRUE(sub3.isInitialized());
        ASSERT_TRUE(sub4.isInitialized());
        ASSERT_TRUE(sub5.isInitialized());

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub.waitDiscovery();
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;

        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs, nmsgs / 16);
        }
        if (recv1 < nmsgs)
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostD += 100. - (recv1 * 100.) / nmsgs;
        }
        if (recv2 < nmsgs)
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostD += 100. - (recv2 * 100.) / nmsgs;
        }
        if (recv3 < nmsgs)
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostD += 100. - (recv3 * 100.) / nmsgs;
        }
        if (recv4 < nmsgs)
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostD += 100. - (recv4 * 100.) / nmsgs;
        }
        if (recv5 < nmsgs)
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostD += 100. - (recv5 * 100.) / nmsgs;
        }
        lostD /= 5.;

    }

    // IS communication
    {
        TestPublisher pub;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;
        TestIS is("config/Data1mb_1_5_Static.xml");

        // Launch IS, they will discover each other
        is.run();

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb");

        ASSERT_TRUE(pub.isInitialized());

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

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub.waitDiscovery();
        eClock::my_sleep(10000); // Wait a moment to match correctly
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;

        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs, nmsgs / 16);
        }
        if (recv1 < nmsgs)
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostI += 100. - (recv1 * 100.) / nmsgs;
        }
        if (recv2 < nmsgs)
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostI += 100. - (recv2 * 100.) / nmsgs;
        }
        if (recv3 < nmsgs)
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostI += 100. - (recv3 * 100.) / nmsgs;
        }
        if (recv4 < nmsgs)
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostI += 100. - (recv4 * 100.) / nmsgs;
        }
        if (recv5 < nmsgs)
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostI += 100. - (recv5 * 100.) / nmsgs;
        }
        lostI /= 5.;

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, Data1mb_3_1_Static)
{
    Data1mbType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub;

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub1_Data1mb");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub2_Data1mb");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub3_Data1mb");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub_Data1mb");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs*3, nmsgs);
        }

        if (recv < nmsgs*3)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostD = 100. - (recv * 100.) / (nmsgs*3);
        }

    }

    // IS communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub;
        TestIS is("config/Data1mb_3_1_Static.xml");

        // Launch IS, they will discover each other
        is.run();

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub1_Data1mb");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 1, &type, "Pub2_Data1mb");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 2, &type, "Pub3_Data1mb");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 5, &type, "Sub_Data1mb");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        eClock::my_sleep(500); // Wait a moment to match correctly

        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs*3, nmsgs);
        }

        if (recv < nmsgs*3)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostI = 100. - (recv * 100.) / (nmsgs*3);
        }

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, Data1mb_1_5_Dynamic)
{
    Data1mbType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb");

        ASSERT_TRUE(pub.isInitialized());

        sub1.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub1_Data1mb");
        sub2.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub2_Data1mb");
        sub3.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub3_Data1mb");
        sub4.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub4_Data1mb");
        sub5.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub5_Data1mb");

        ASSERT_TRUE(sub1.isInitialized());
        ASSERT_TRUE(sub2.isInitialized());
        ASSERT_TRUE(sub3.isInitialized());
        ASSERT_TRUE(sub4.isInitialized());
        ASSERT_TRUE(sub5.isInitialized());

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub.waitDiscovery();
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;

        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs, nmsgs / 16);
        }
        if (recv1 < nmsgs)
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostD += 100. - (recv1 * 100.) / nmsgs;
        }
        if (recv2 < nmsgs)
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostD += 100. - (recv2 * 100.) / nmsgs;
        }
        if (recv3 < nmsgs)
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostD += 100. - (recv3 * 100.) / nmsgs;
        }
        if (recv4 < nmsgs)
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostD += 100. - (recv4 * 100.) / nmsgs;
        }
        if (recv5 < nmsgs)
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostD += 100. - (recv5 * 100.) / nmsgs;
        }
        lostD /= 5.;

    }

    // IS communication
    {
        TestPublisher pub;
        TestSubscriber sub1;
        TestSubscriber sub2;
        TestSubscriber sub3;
        TestSubscriber sub4;
        TestSubscriber sub5;
        TestIS is("config/Data1mb_1_5_Dynamic.xml");

        // Launch IS, they will discover each other
        is.run();

        pub.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub_Data1mb");

        ASSERT_TRUE(pub.isInitialized());

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

        // Wait for discovery.
        sub1.waitDiscovery();
        sub2.waitDiscovery();
        sub3.waitDiscovery();
        sub4.waitDiscovery();
        sub5.waitDiscovery();
        pub.waitDiscovery();
        eClock::my_sleep(10000); // Wait a moment to match correctly
        size_t recv1 = 0;
        size_t recv2 = 0;
        size_t recv3 = 0;
        size_t recv4 = 0;
        size_t recv5 = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;

        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub.send();
                eClock::my_sleep(0);
            }

            recv1 = sub1.block_for_at_least(nmsgs, nmsgs);
            recv2 = sub2.block_for_at_least(nmsgs, nmsgs / 2);
            recv3 = sub3.block_for_at_least(nmsgs, nmsgs / 4);
            recv4 = sub4.block_for_at_least(nmsgs, nmsgs / 8);
            recv5 = sub5.block_for_at_least(nmsgs, nmsgs / 16);
        }
        if (recv1 < nmsgs)
        {
            std::cout << "1- Received " << recv1 << " samples!!" << std::endl;
            lostI += 100. - (recv1 * 100.) / nmsgs;
        }
        if (recv2 < nmsgs)
        {
            std::cout << "2- Received " << recv2 << " samples!!" << std::endl;
            lostI += 100. - (recv2 * 100.) / nmsgs;
        }
        if (recv3 < nmsgs)
        {
            std::cout << "3- Received " << recv3 << " samples!!" << std::endl;
            lostI += 100. - (recv3 * 100.) / nmsgs;
        }
        if (recv4 < nmsgs)
        {
            std::cout << "4- Received " << recv4 << " samples!!" << std::endl;
            lostI += 100. - (recv4 * 100.) / nmsgs;
        }
        if (recv5 < nmsgs)
        {
            std::cout << "5- Received " << recv5 << " samples!!" << std::endl;
            lostI += 100. - (recv5 * 100.) / nmsgs;
        }
        lostI /= 5.;

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

TEST_F(Benchmark, Data1mb_3_1_Dynamic)
{
    Data1mbType type;
    const uint16_t nmsgs = 10000;
    double direct, indirect;
    float lostD = 0.0;
    float lostI = 0.0;

    // Direct communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub;

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub1_Data1mb");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub2_Data1mb");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub3_Data1mb");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 0, &type, "Sub_Data1mb");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(direct);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs*3, nmsgs);
        }

        if (recv < nmsgs*3)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostD = 100. - (recv * 100.) / (nmsgs*3);
        }

    }

    // IS communication
    {
        TestPublisher pub1;
        TestPublisher pub2;
        TestPublisher pub3;
        TestSubscriber sub;
        TestIS is("config/Data1mb_3_1_Dynamic.xml");

        // Launch IS, they will discover each other
        is.run();

        pub1.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 0, &type, "Pub1_Data1mb");
        pub2.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 1, &type, "Pub2_Data1mb");
        pub3.init(UDP, RELIABLE_RELIABILITY_QOS, 5, 10, "Data1mbTopic", 2, &type, "Pub3_Data1mb");

        ASSERT_TRUE(pub1.isInitialized());
        ASSERT_TRUE(pub2.isInitialized());
        ASSERT_TRUE(pub3.isInitialized());

        sub.init(UDP, RELIABLE_RELIABILITY_QOS, "Data1mbTopic", 5, &type, "Sub_Data1mb");

        ASSERT_TRUE(sub.isInitialized());

        // Wait for discovery.
        sub.waitDiscovery();
        pub1.waitDiscovery();
        pub2.waitDiscovery();
        pub3.waitDiscovery();
        eClock::my_sleep(500); // Wait a moment to match correctly

        size_t recv = 0;
        std::cout << "Start sending " << nmsgs << " samples." << std::endl;
        {
            Chrono chrono(indirect);
            for(uint16_t count = 0; count < nmsgs; ++count)
            {
                pub1.send();
                eClock::my_sleep(0);
                pub2.send();
                eClock::my_sleep(0);
                pub3.send();
                eClock::my_sleep(0);
            }

            recv = sub.block_for_at_least(nmsgs*3, nmsgs);
        }

        if (recv < nmsgs*3)
        {
            std::cout << "Received " << recv << " samples!!" << std::endl;
            lostI = 100. - (recv * 100.) / (nmsgs*3);
        }

        is.stop();
    }

    std::cout << "Results with " << nmsgs << " samples: " << std::endl;
    std::cout << "Direct communication:  " << direct << " ms. (" << lostD << "% lost)" << std::endl;
    std::cout << "Communication with IS: " << indirect << " ms. (" << lostI << "% lost)" << std::endl;
    std::cout << "Difference: " << indirect - direct << " ms. ("
        << (100. * (indirect - direct)) / direct << " %)" << std::endl;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}