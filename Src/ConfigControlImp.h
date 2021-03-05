#include "nlohmann/json.hpp"
#include "utilities.h"

namespace JSON = nlohmann;

namespace Tooling
{
const JSON::json DEFAULT_SERVER_INFO = R"({
        "ServerInfo":{
            "IP":"192.168.1.1",
            "port":80
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
    ConfigControlImp();

    bool getConfig(std::string Name, JSON::json& Config);

    bool setConfig(std::string Name, JSON::json Config);

private:
    std::string m_sConfigFilePath;
    JSON::json m_jsonConfig;
};


}   // namespace Tooling