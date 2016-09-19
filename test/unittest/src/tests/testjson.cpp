#include "testjson.h"

#include <nap/logger.h>
#include <rapidjson/encodings.h>
#include <rapidjson/document.h>

using namespace nap;

bool testJson()
{
    using namespace rapidjson;

    std::string json = "{\"project\":\"rapidjson\",\"stars\":10}";
    GenericDocument <UTF8<>> d;
    d.Parse(json.c_str());

    int stars = d["stars"].GetInt();
    Logger::debug(std::to_string(stars));

    return true;
}