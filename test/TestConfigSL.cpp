#include "ConfigControlImp.h"
#include "Infra/FactoryTemplate.h"
#include "catch2/catch.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "utilities.h"

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <unistd.h>
using namespace JSON;
using namespace Tooling;
using namespace std;

TEST_CASE("test write and read a Json file", "[ConfigSL]")
{
    string path = CONFIG_FILE_PATH + "write.json";
    ofstream o(path);
    o << setw(4) << DEFAULT_SERVER_INFO << endl;

    json readIn;
    std::ifstream in(path);
    in >> readIn;
    REQUIRE(readIn == DEFAULT_SERVER_INFO);
    remove(path.c_str());
}

TEST_CASE("test create config control instance", "[ConfigSL]")
{
    string path = CONFIG_FILE_PATH + "Config.json";
    if (access(path.c_str(), F_OK) != -1)
        remove(path.c_str());
    SECTION("Test default config")
    {
        ConfigControlImp configControl;
        json serverDefaultInfo;
        bool result = configControl.getConfig("ServerInfo", serverDefaultInfo);
        REQUIRE(result);
        REQUIRE(DEFAULT_SERVER_INFO["ServerInfo"] == serverDefaultInfo);
    }

    SECTION("Test config from file")
    {
        json testConfig = DEFAULT_SERVER_INFO;
        testConfig["ServerInfo"]["IP"] = "0.0.0.0";
        ofstream o(path);
        o << setw(4) << testConfig << endl;
        ConfigControlImp configControl;
        json serverInfo;
        bool result = configControl.getConfig("ServerInfo", serverInfo);
        REQUIRE(result);
        REQUIRE(testConfig["ServerInfo"] == serverInfo);
    }

    SECTION("Test set config to file")
    {
        ConfigControlImp configControl;
        json serverInfo = DEFAULT_SERVER_INFO["ServerInfo"];
        serverInfo["IP"] = "0.0.0.0";
        bool result = configControl.setConfig("ServerInfo", serverInfo);
        REQUIRE(result);
        json readIn;
        std::ifstream in(path);
        in >> readIn;
        REQUIRE(readIn["ServerInfo"] == serverInfo);
    }
    remove(path.c_str());
}