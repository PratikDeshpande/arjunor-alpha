#include <memory>
#include <sys/socket.h> 
#include <iostream>
#include <string.h>
#include "eval.h"
#include <unistd.h>
#include <sstream>

namespace comm {

    std::shared_ptr<cmd::RedisCommand> read_command(int new_fd);
    void send_command(int new_fd, std::shared_ptr<cmd::RedisCommand> command, std::shared_ptr<store::ObjectStore> object_store);
    void send_error(int new_fd, std::string error_message);
    void handle_client_request(int new_fd, char* incoming_connection_details, std::shared_ptr<store::ObjectStore> object_store);

}
