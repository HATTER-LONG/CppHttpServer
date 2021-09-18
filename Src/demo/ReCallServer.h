#pragma once
#include <string>


namespace ToolKit
{
class ReCallServer
{
public:
    static ReCallServer* instance()
    {
        static ReCallServer* svr = new ReCallServer {};
        return svr;
    }

public:
    void run();

private:
    ReCallServer();

private:
    std::string m_sIpAddr;
    int m_iPort;
    int m_iThreadNums;
};
}   // namespace ToolKit