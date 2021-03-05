#pragma once

#include <string>
namespace Tooling
{
/**
 * @brief Include directories for standard headers
 *
 */
#ifdef CONFIGFILE_FULLPATH
std::string const CONFIG_FILE_PATH(CONFIGFILE_FULLPATH);
#else
std::string const CONFIG_FILE_PATH;
#endif
}   // namespace Tooling