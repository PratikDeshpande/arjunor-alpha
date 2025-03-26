#include "eval.h"

namespace eval {
    void eval_ping(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {

        if (arguments.size() >= 2) {
            std::cout << "Error: Too many arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'ping' command");
        }

        if (arguments.size() == 1) {
            std::vector<std::byte> response_byte_vector = resp::encode_bulk_string(arguments[0]);
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        } else {
            std::vector<std::byte> response_byte_vector = resp::encode_simple_string("PONG");
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        }

    }

    void eval_set(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        if (arguments.size() != 2) { // TODO: Add support for TTL option
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'set' command");
        } else {
            
            auto key = arguments[0];
            auto value = arguments[1];
            auto object = std::make_shared<store::Object>();

            for (auto strChar: value) {
                object->data.push_back(std::byte(strChar));
            }
            object->timestamp = std::time(nullptr);
            object_store->insert(key, object);
            
            std::vector<std::byte> response_byte_vector = resp::encode_simple_string("OK");
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
                
        }
    }

    void eval_get(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        if (arguments.size() != 1) {
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'get' command");
        } else {
            auto key = arguments[0];
            auto object = object_store->get(key);
            if (object != nullptr) {
                auto object_data = object->data;
                std::string data_string(  reinterpret_cast<const char*>(object_data.data()), reinterpret_cast<const char*>(object_data.data()  + object_data.size() ));

                std::vector<std::byte> response_byte_vector = resp::encode_bulk_string(data_string);
                auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
                if (bytes_sent <= 0) {
                    std::cout << "Error sending bytes" << std::endl;
                    throw std::invalid_argument("Error sending bytes");
                }
                    

            } else {
                std::vector<std::byte> response_byte_vector = resp::encode_null();
                auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
                if (bytes_sent <= 0) {
                    std::cout << "Error sending bytes" << std::endl;
                    throw std::invalid_argument("Error sending bytes");
                }
            }
        }
    }

    void eval_and_respond(std::shared_ptr<cmd::RedisCommand> command, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {

        if (command->name == cmd::CommandName::Ping) {
            eval_ping(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::Set) {
            eval_set(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::Get) {
            eval_get(command->arguments, new_fd, object_store);
        } else {
            eval_ping(command->arguments, new_fd, object_store);
        }


    }
}