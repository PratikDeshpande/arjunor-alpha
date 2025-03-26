#include <memory>
#include "cmd.h"
#include "store.h"
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include "resp.h"

namespace eval {
    void eval_ping(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store);
    void eval_set(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store);
    void eval_get(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store);
    void eval_and_respond(std::shared_ptr<cmd::RedisCommand> command, int new_fd, std::shared_ptr<store::ObjectStore> object_store);
}