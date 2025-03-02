// #include "../communication/include/bus_manager.h"

// int main()
// {
//     std::vector<uint32_t> ids;
//     uint32_t limit;
//     BusManager* manager = BusManager::getInstance(ids, limit);
//     manager->startConnection();
//     while(true);
//     return 0;
// }
#include "../communication/include/bus_manager.h"
#include "../logger/logger.h"

class g{
    public:
    static logger guiLogger;
    g();
};
logger g::guiLogger("gui");
g::g(){
guiLogger.logMessage(logger::LogLevel::DEBUG,
                         "Failed to run script and retrieve JSON file path.");}
int main()
{
    g();
    std::vector<uint32_t> ids;
    uint32_t limit;
    BusManager* manager = BusManager::getInstance(ids, limit);
    manager->startConnection();
    while(true);
    return 0;
}

