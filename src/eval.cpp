#include "eval.h"
#include <sys/socket.h>
#include <iostream>
#include <sstream>

namespace eval {
    void eval_ping(std::vector<std::string> arguments, int new_fd) {

        if (arguments.size() >= 2) {
            std::cout << "Error: Too many arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'ping' command");
        }

        if (arguments.size() == 1) {
            // TODO: Use encode function to convert to RESP format
            std::ostringstream response_stream;
            response_stream << "$" << arguments[0].size() << "\r\n" << arguments[0] << "\r\n";
            std::string response = response_stream.str();
            auto char_string = response.c_str();
            auto byte_string = (std::byte*)char_string;
            printf("response size: %li\n", response.size());
            auto bytes_sent = send(new_fd, byte_string, response.size(), 0);

            printf("bytes sent: %li\n", bytes_sent);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        } else {
            // TODO: Use encode function to convert to RESP format
            std::string response = "+PONG\r\n";
            auto char_string = response.c_str();
            auto byte_string = (std::byte*)char_string;
            printf("response size: %li\n", response.size());
            auto bytes_sent = send(new_fd, byte_string, response.size(), 0);

            printf("bytes sent: %li\n", bytes_sent);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        }

    }

    void eval_and_respond(std::shared_ptr<cmd::RedisCommand> command, int new_fd) {

        if (command->name == cmd::CommandName::Ping) {
            eval_ping(command->arguments, new_fd);
        } else {
            eval_ping(command->arguments, new_fd);
        }


    }
}