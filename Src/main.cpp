#include "HttpServer.h"
#include "spdlog/spdlog.h"

using namespace ToolKit;
using namespace spdlog;

int main(int argc, char* argv[])
{
    HttpServer::instance()->run();
    return 0;
}