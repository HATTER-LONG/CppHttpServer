#include "ConfigControlImp.h"
#include "HttpServer.h"
#include "ThreadPool.h"
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstring>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <netinet/in.h>
#include <spdlog/fmt/bundled/format.h>
#include <sys/socket.h>
using namespace spdlog;
using namespace std;

static void onReadCb(struct bufferevent* Bev, void* Ctx)
{
    /* 获取bufferevent中的读和写的指针 */
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer* input = bufferevent_get_input(Bev);
    struct evbuffer* output = bufferevent_get_output(Bev);
    /* 把读入的数据全部复制到写内存中 */
    /* Copy all the data from the input buffer to the output buffer. */
    evbuffer_add_buffer(output, input);
}
static void echoEventCb(struct bufferevent* Bev, short Events, void* Ctx)
{
    if (Events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (Events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
    {
        bufferevent_free(Bev);
    }
}

static void onAcceptCb(struct evconnlistener* Listener, evutil_socket_t Fd, struct sockaddr* Addr, int Socklen, void* Ctx)
{
    /* 初始化一个 bufferevent 用于数据的写入和读取，首先需要从 Listener 中获取 event_base */
    /* We got a new connection! Set up a bufferevent for it. */

    struct event_base* base = evconnlistener_get_base(Listener);
    info("{} base ptr [{}]", __FUNCTION__, fmt::ptr(base));
    struct bufferevent* bev = bufferevent_socket_new(base, Fd, 0);

    /*设置 bufferevent 的回调函数，这里设置了读和事件的回调函数*/
    bufferevent_setcb(bev, onReadCb, NULL, echoEventCb, NULL);
    /* 启用该 buffevent 写和读 */
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void acceptErrorCb(struct evconnlistener* Listener, void* Ctx)
{
    struct event_base* base = evconnlistener_get_base(Listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr,
        "Got an error %d (%s) on the listener. "
        "Shutting down.\n",
        err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

namespace Tooling
{
HttpServer::HttpServer()
{
    info("initialization httpServer >>>>>");

    JSON::json serverConfigInfo;
    ConfigControlImp::instance()->getConfig("ServerInfo", serverConfigInfo);
    info("Load ServerInfo Config is \n{}", serverConfigInfo.dump(4).c_str());
    m_sIpAddr = serverConfigInfo["IP"].get<string>();
    m_iPort = serverConfigInfo["Port"].get<int>();
    m_iThreadNums = serverConfigInfo["threads"].get<int>();

    m_threadpool = new ThreadPool { static_cast<size_t>(m_iThreadNums) };
}

void HttpServer::run()
{
    struct event_base* base = event_base_new();
    info("{} base ptr [{}]", __FUNCTION__, fmt::ptr(base));
    if (!base)
    {
        warn("open event base failed");
        return;
    }
    int ret = evthread_use_pthreads();
    if (ret != 0)
    {
        warn("set event use pthread failed");
        return;
    }
    struct sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(m_iPort);
    serveraddr.sin_addr.s_addr = inet_addr(m_sIpAddr.c_str());

    auto* listener = evconnlistener_new_bind(
        base, onAcceptCb, NULL, LEV_OPT_REUSEABLE, 10, (const struct sockaddr*)(&serveraddr), sizeof(serveraddr));
    if (!listener)
    {
        warn("Listener init error\n");
        return;
    }
    /* 设置 Listen 错误回调函数 */
    evconnlistener_set_error_cb(listener, acceptErrorCb);
    /* 开始accept进入循环 */
    event_base_dispatch(base);

    event_base_free(base);
    evconnlistener_free(listener);
}
}   // namespace Tooling
