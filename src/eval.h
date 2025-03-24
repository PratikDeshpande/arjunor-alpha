#include <memory>
#include "cmd.h"

namespace eval {
    void eval_ping(std::vector<std::string> arguments, int new_fd);
    void eval_set(std::vector<std::string> arguments, int new_fd);
    void eval_get(std::vector<std::string> arguments, int new_fd);
    void eval_and_respond(std::shared_ptr<cmd::RedisCommand> command, int new_fd);
}