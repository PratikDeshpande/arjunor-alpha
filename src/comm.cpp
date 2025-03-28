#include "comm.h"

namespace comm {


// Read from client. This is a blocking system call
std::shared_ptr<cmd::RedisCommand> read_command(int new_fd) {
    std::byte buffer[512]; // TODO: If bigger than 512 bytes, loop until you read EOF. Clear memory?
    auto bytes_read = recv(new_fd, buffer, 512, 0);
    if (bytes_read <= 0) {
        std::cout << "Error reading incoming bytes" << std::endl;
        throw std::invalid_argument("Error: No data in buffer");
    }
    std::vector<std::byte> command(buffer, buffer + bytes_read);
    memset(&buffer, 0, sizeof buffer);
    std::string command_string(reinterpret_cast<const char*>(command.data()), command.size());
    std::cout << "Incoming command: " << command_string << std::endl;

    // TODO: Extract to helper function
    auto protocol_message = resp::decode(command).first;
    if (protocol_message->protocol_message_type != resp::ProtocolMessageType::Array) {
        std::cout << "Error: Expected Array type" << std::endl;
        throw std::invalid_argument("Error: Expected Array type");
    }
    auto data = protocol_message->data;
    auto vectors = std::get<std::vector<std::shared_ptr<resp::ProtocolMessage>>>(data);
    if (vectors.size() == 0) {
        std::cout << "Error: Expected at least one element in array" << std::endl;
        throw std::invalid_argument("Error: Expected at least one element in array");
    }
    for (auto vec: vectors) {
        if (vec->protocol_message_type != resp::ProtocolMessageType::BulkString) {
            std::cout << "Error: Expected BulkString type" << std::endl;
            throw std::invalid_argument("Error: Expected BulkString type");
        }
    }
    auto command_name = std::get<std::string>(vectors[0]->data);
    cmd::CommandName command_name_enum;
    // TODO: extract to helper function to convert string to enum
    if (command_name == "PING") {
        command_name_enum = cmd::CommandName::Ping;
    } else if (command_name == "SET") {
        command_name_enum = cmd::CommandName::Set;
    } else if (command_name == "GET") {
        command_name_enum = cmd::CommandName::Get;
    } else if (command_name == "VECTORINDEX") {
        command_name_enum = cmd::CommandName::VectorIndex;
    } else if (command_name == "UPSERT") {
        command_name_enum = cmd::CommandName::Upsert;
    } else if (command_name == "SEARCH") {
        command_name_enum = cmd::CommandName::Search;
    } else {
        std::cout << "Error: Unknown command" << std::endl;
        throw std::invalid_argument("Error: Unknown command");
    }

    std::vector<std::string> arguments;
    for(int i = 1; i < vectors.size(); i++) {
        arguments.push_back(std::get<std::string>(vectors[i]->data));
    }
    std::shared_ptr<cmd::RedisCommand> result = std::make_shared<cmd::RedisCommand>();
    result->name = command_name_enum;
    result->arguments = arguments;
    return result;
}

void send_error(int new_fd, std::string error_message) {
    std::ostringstream error_message_stream;
    error_message_stream << "-" << error_message << "\r\n";
    std::string response = error_message_stream.str();


    auto char_string = response.c_str();
    auto byte_string = (std::byte*)char_string;
    auto bytes_sent = send(new_fd, byte_string, response.size(), 0);
}


void send_command(int new_fd, std::shared_ptr<cmd::RedisCommand> command, std::shared_ptr<store::ObjectStore> object_store) {
    try {
        eval::eval_and_respond(command, new_fd, object_store);
    } catch (std::invalid_argument& e) {
        auto error_message = std::string(e.what());
        printf("Error sending command: %s\n", e.what());
        send_error(new_fd, error_message);
    }
}

// maybe keep this elsewhere
void handle_client_request(int new_fd, char* incoming_connection_details, std::shared_ptr<store::ObjectStore> object_store) {
    std::shared_ptr<cmd::RedisCommand> command;
    try {
        command = read_command(new_fd);
    } catch (std::invalid_argument& e) {
        // TODO: ERROR: Keep receiving messages after socket is closed
        printf("Error reading command: %s\n", e.what());
        close(new_fd);
        printf("client session with address %s disconnected\n", incoming_connection_details);
        return;
    }
    send_command(new_fd, command, object_store);
}

}