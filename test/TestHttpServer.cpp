#include "HttpServer.h"
#include "catch2/catch.hpp"
using namespace Catch;
using namespace std;
using namespace Tooling;

TEST_CASE("Test create a HttpServer", "[HttpServer]")
{
    HttpServer::instance();
}
