#pragma once
#include <string>

class ThreadPool;

namespace Tooling
{
class HttpServer
{
public:
    static HttpServer* instance()
    {
        static HttpServer* svr = new HttpServer {};
        return svr;
    }

public:
    void run();

private:
    HttpServer();

private:
    std::string m_sIpAddr;
    int m_iPort;
    int m_iThreadNums;
    ThreadPool* m_threadpool;
};
}   // namespace Tooling