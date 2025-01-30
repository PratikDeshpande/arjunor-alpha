#include <string>
#include <vector>

namespace cmd {
    enum CommandName { Ping };
    struct RedisCommand {
        CommandName name;
        std::vector<std::string> arguments;
    };
}