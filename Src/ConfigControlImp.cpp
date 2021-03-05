#include "ConfigControlImp.h"
#include "utilities.h"

#include <fstream>
#include <iomanip>
#include <unistd.h>
using namespace std;
namespace Tooling
{
ConfigControlImp::ConfigControlImp()
        : m_sConfigFilePath(CONFIG_FILE_PATH + "Config.json")
        , m_jsonConfig(DEFAULT_SERVER_INFO)
{
    if (access(m_sConfigFilePath.c_str(), F_OK) != -1)
    {
        ifstream in(m_sConfigFilePath);
        in >> m_jsonConfig;
    }
}
bool ConfigControlImp::getConfig(std::string Name, JSON::json& Config)
{
    if (m_jsonConfig.find(Name) != m_jsonConfig.end())
    {
        Config = m_jsonConfig[Name];
        return true;
    }
    return false;
}
bool ConfigControlImp::setConfig(std::string Name, JSON::json Config)
{
    m_jsonConfig[Name] = Config;
    ofstream o(m_sConfigFilePath);
    o << setw(4) << DEFAULT_SERVER_INFO << endl;
    return true;
}
}   // namespace Tooling