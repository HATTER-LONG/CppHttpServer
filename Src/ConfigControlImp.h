#include "nlohmann/json.hpp"
#include "utilities.h"

namespace JSON = nlohmann;

namespace ToolKit
{
const JSON::json DEFAULT_SERVER_INFO = R"({
        "ServerInfo":{
            "IP":"192.168.1.1",
            "Port":80,
            "threads":8,
            "ReadTimeOut_S":100,
            "WriteTimeOut_S":100
        }
    })"_json;

class ConfigControlImp
{
public:
    static ConfigControlImp* instance()
    {
        static ConfigControlImp* o = new ConfigControlImp {};
        return o;
    }

    bool getConfig(std::string Name, JSON::json& Config);

    bool setConfig(std::string Name, JSON::json Config);

private:
    ConfigControlImp();
    ~ConfigControlImp() = default;
    ConfigControlImp(const ConfigControlImp&) = delete;
    const ConfigControlImp& operator=(const ConfigControlImp&) = delete;

    std::string m_sConfigFilePath;
    JSON::json m_jsonConfig;
};


}   // namespace ToolKit