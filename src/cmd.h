#include <string>
#include <vector>

namespace cmd {
    enum CommandName { Ping, Set, Get };
    struct RedisCommand {
        CommandName name;
        std::vector<std::string> arguments;
    };
}