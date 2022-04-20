
#include <chrono>

#include "util/timestamp.h"

std::string Util::getCurrentDate()
{
    std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    char buf[20] = {0};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %T", std::localtime(&now));

    return std::string(buf);
}