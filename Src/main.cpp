#include "ConfigControlImp.h"
#include "spdlog/spdlog.h"

using namespace Tooling;
using namespace spdlog;

int main(int argc, char* argv[])
{
    JSON::json serverInfo;
    ConfigControlImp::instance()->getConfig("ServerInfo", serverInfo);
    info("ServerInfo config is \n{}", serverInfo.dump(4).c_str());
    return 0;
}