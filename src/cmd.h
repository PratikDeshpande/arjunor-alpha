#include <string>
#include <vector>

namespace cmd {
    enum CommandName { Ping, Set, Get, VectorIndex, Upsert, Search };
    struct RedisCommand {
        CommandName name;
        std::vector<std::string> arguments;
    };
}