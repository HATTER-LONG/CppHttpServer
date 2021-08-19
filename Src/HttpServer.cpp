#include "HttpServer.h"

#include "ConfigControlImp.h"
#include "Infra/ThreadPool.h"
#include "spdlog/spdlog.h"

#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_compat.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace spdlog;
using namespace std;

static int socketReadTimeoutSeconds = 100;
static int socketWriteTimeoutSeconds = 100;
ThreadPool* threadPool;
/**
 * Struct to carry around connection (client)-specific data.
 */
typedef struct Client
{
    /* The event_base for this client. */
    struct event_base* m_evbase;

    /* The bufferedevent for this client. */
    struct bufferevent* m_bufEv;

} client_t;

static void onWrite(struct bufferevent* Bev, void* Arg) { }

static void onReadCb(struct bufferevent* Bev, void* Ctx)
{
    struct evbuffer* input = bufferevent_get_input(Bev);
    struct evbuffer* output = bufferevent_get_output(Bev);
    evbuffer_add_buffer(output, input);
}
static void echoEventCb(struct bufferevent* Bev, short Events, void* Ctx)
{
    info("{} receive event[{}]", __FUNCTION__, Events);
}

static int threadNums = 0;
static vector<struct Client*> clientVector;
static int baseIndex = 0;

static void closeAndFreeClient(client_t* Client)
{
    for (auto iter = clientVector.begin(); iter != clientVector.end(); iter++)
    {
        if (*iter.base() == Client)
        {
            clientVector.erase(iter);
            break;
        }
    }
    if (Client != NULL)
    {
        if (Client->m_evbase != NULL)
        {
            event_base_free(Client->m_evbase);
            Client->m_evbase = NULL;
        }
        free(Client);
    }
}
static void serverJobFunction(struct Client* Client)
{
    event_base_dispatch(Client->m_evbase);
    closeAndFreeClient(Client);
}
static void onAcceptCb(struct evconnlistener* Listener, evutil_socket_t Fd, struct sockaddr* Addr, int Socklen, void* Ctx)
{
    struct Client* client = NULL;
    bool addThreadPool = false;
    if (clientVector.size() < threadNums)
    {
        client = new Client;
        memset(client, 0, sizeof(*client));
        if ((client->m_evbase = event_base_new()) == NULL)
        {
            warn("client event_base creation failed");
            closeAndFreeClient(client);
            return;
        }
        clientVector.push_back(client);
        addThreadPool = true;
    }
    else
    {
        client = clientVector[baseIndex];
        baseIndex = ++baseIndex == clientVector.size() ? 0 : baseIndex;
    }

    if ((client->m_bufEv = bufferevent_socket_new(client->m_evbase, Fd, BEV_OPT_CLOSE_ON_FREE)) == NULL)
    {
        warn("client bufferevent creation failed");
        closeAndFreeClient(client);
        return;
    }
    /*设置 bufferevent 的回调函数，这里设置了读和事件的回调函数*/
    bufferevent_setcb(client->m_bufEv, onReadCb, onWrite, echoEventCb, NULL);

    bufferevent_base_set(client->m_evbase, client->m_bufEv);
    struct timeval readTimeOut;
    readTimeOut.tv_sec = socketReadTimeoutSeconds;
    readTimeOut.tv_usec = 0;

    struct timeval rwriteTimeOut;
    rwriteTimeOut.tv_sec = socketWriteTimeoutSeconds;
    rwriteTimeOut.tv_usec = 0;
    bufferevent_set_timeouts(client->m_bufEv, &readTimeOut, &rwriteTimeOut);

    /*
     * We have to enable it before our callbacks will be called.
     */
    bufferevent_enable(client->m_bufEv, EV_READ | EV_WRITE);
    if (addThreadPool)
        threadPool->enqueue(serverJobFunction, client);
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
    threadNums = m_iThreadNums;
    socketReadTimeoutSeconds = serverConfigInfo["ReadTimeOut_S"].get<int>();
    socketWriteTimeoutSeconds = serverConfigInfo["WriteTimeOut_S"].get<int>();
    threadPool = new ThreadPool { static_cast<size_t>(m_iThreadNums) };
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
    evconnlistener_set_error_cb(listener, acceptErrorCb);
    event_base_dispatch(base);

    event_base_free(base);
    evconnlistener_free(listener);
}
}   // namespace Tooling
