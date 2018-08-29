#include "TestIS.h"

TestIS::TestIS(std::string config)
    : m_Manager(nullptr)
{
    m_Config = config;
}

void TestIS::run()
{
    if (m_Manager != nullptr)
    {
        delete m_Manager;
    }
    m_Manager = new ISManager(m_Config);
}

void TestIS::stop()
{
    delete m_Manager;
    m_Manager = nullptr;
}

TestIS::~TestIS()
{
    stop();
}
