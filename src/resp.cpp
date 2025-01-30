#include "resp.h"
#include <iostream>

namespace resp {

std::byte carriage_return_character = static_cast<std::byte>('\r');
std::byte zero_decimal_character = static_cast<std::byte>('0');;
std::byte nine_decimal_character = static_cast<std::byte>('9');

std::pair<int, int> read_length(const std::vector<std::byte>& buffer) {
    int position = 0;
    int length = 0;
    for (auto byte: buffer) {
        if (byte >= zero_decimal_character && byte <= nine_decimal_character) {
            length = length*10 + (static_cast<int>(byte) - '0');

        } else {
            break;
        }
        position++;
    }
    return std::make_pair(length, position + 2);
}

std::pair<std::string, int> read_simple_string(const std::vector<std::byte>& buffer) {
    int position;
    for (position = 1; buffer[position] != carriage_return_character; position++) {
    }
    std::string result(  reinterpret_cast<const char*>(buffer.data() + 1), reinterpret_cast<const char*>(buffer.data() + position));
    return std::make_pair(result, position + 2);
}

std::pair<std::string, int> read_error(const std::vector<std::byte>& buffer) {
    return read_simple_string(buffer);
}

std::pair<std::int64_t, int> read_int_64(const std::vector<std::byte>& buffer) {
    int64_t result = 0;
    int position;
    for (position = 1; buffer[position] != carriage_return_character; position++) {
        result = result * 10 + (static_cast<int64_t>(buffer[position]) - '0');
    }
    return std::make_pair(result, position + 2);

}

std::pair<std::string, int> read_bulk_string(const std::vector<std::byte>& buffer) {
    int position = 1;
    auto length_delta_pair = read_length(std::vector<std::byte>(buffer.begin() + position, buffer.end()));
    auto length = length_delta_pair.first;
    auto delta = length_delta_pair.second;
    position = position + delta;
    std::string result(  reinterpret_cast<const char*>(buffer.data() + position), reinterpret_cast<const char*>(buffer.data() + position + length ));
    return std::make_pair(result, position + length + 2);
}

std::pair<std::shared_ptr<ProtocolMessage>, int> read_array(const std::vector<std::byte>& buffer) {
    int position = 1;
    auto count_delta_pair = read_length(std::vector<std::byte>(buffer.begin() + position, buffer.end()));
    auto count = count_delta_pair.first;
    auto delta = count_delta_pair.second;
    position = position + delta;
    std::vector<std::shared_ptr<ProtocolMessage>> elements;
    for (int i=0; i < count; i++) {
        // TODO: wrap around try/catch and propagate error up
        auto elements_pair = decode_one(std::vector<std::byte>(buffer.begin() + position, buffer.end()));
        auto element = elements_pair.first;
        auto element_delta = elements_pair.second;
        elements.push_back(element);
        position = position + element_delta;
    }
    std::shared_ptr<ProtocolMessage> result = std::make_shared<ProtocolMessage>();
    result->protocol_message_type = ProtocolMessageType::Array;
    result->data = elements; // TODO: use std::copy instead?  std::copy(elements.begin(), elements.end(), result->data);
    return std::make_pair(result, position);
}

std::pair<std::shared_ptr<ProtocolMessage>, int> decode_one(const std::vector<std::byte>& buffer) {
    if (buffer.size() == 0) {
        std::cout << "Error: No Data in buffer" << std::endl;
        throw std::invalid_argument("Error: No Data in buffer");
    }

    auto first_character = static_cast<char>(buffer[0]);
    std::shared_ptr<ProtocolMessage> result = std::make_shared<ProtocolMessage>();

    // Conditionals within cases in a C++ switch statement don't have a unique scope. 
    // results in : transfer of control bypasses initialization of <x>
    // Unpredictable behavior if objects are instantiated. Using if/else

    // TODO: See if you can automate this with templates
    if (first_character == '+') {
        std::shared_ptr<ProtocolMessage> result = std::make_shared<ProtocolMessage>();
        auto simple_string_pair = read_simple_string(buffer);
        result->protocol_message_type = ProtocolMessageType::SimpleString;
        result->data = simple_string_pair.first;
        return std::make_pair(result, simple_string_pair.second);
    } else if (first_character == '-') {
        std::shared_ptr<ProtocolMessage> result = std::make_shared<ProtocolMessage>();
        auto simple_error_pair = read_error(buffer);
        result->protocol_message_type = ProtocolMessageType::SimpleError;
        result->data = simple_error_pair.first;
        return std::make_pair(result, simple_error_pair.second);
    } else if (first_character == ':') {
        std::shared_ptr<ProtocolMessage> result = std::make_shared<ProtocolMessage>();
        auto int_64_pair = read_int_64(buffer);
        result->protocol_message_type = ProtocolMessageType::Integer;
        result->data = int_64_pair.first;
        return std::make_pair(result, int_64_pair.second);
    } else if (first_character == '$') {
        std::shared_ptr<ProtocolMessage> result = std::make_shared<ProtocolMessage>();
        auto bulk_string_pair = read_bulk_string(buffer);
        result->protocol_message_type = ProtocolMessageType::BulkString;
        result->data = bulk_string_pair.first;
        return std::make_pair(result, bulk_string_pair.second);
    } else if (first_character == '*') {
        return read_array(buffer);
    } else {
        std::cout << "Error: Unrecognized Symbol" << std::endl;
        throw std::invalid_argument("Error: Unrecognized Symbol");
    }
}

std::pair<std::shared_ptr<ProtocolMessage>, int> decode(const std::vector<std::byte>& buffer) {
    if (buffer.size() == 0) {
        std::cout << "Error: No Data in buffer" << std::endl;
        throw std::invalid_argument("Error: No Data in buffer"); // TODO: This may not need to be a thrown error if we iteratively reach this point
    }
    auto result = decode_one(buffer);
    return result;
}
}