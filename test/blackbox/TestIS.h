#include <ISManager.h>
#include <string>

class TestIS
{
public:
    TestIS(std::string config);
    ~TestIS();
    void run();
    void stop();
private:
    ISManager *m_Manager;
    std::string m_Config;
};
