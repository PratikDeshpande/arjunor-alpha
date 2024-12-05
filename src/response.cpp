#include "response.h"
#include <string.h>
#include <vector>


namespace response {

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

std::pair<ProtocolMessage, int> read_array(const std::vector<std::byte>& buffer) {
    int position = 1;
    auto count_delta_pair = read_length(std::vector<std::byte>(buffer.begin() + position, buffer.end()));
    auto count = count_delta_pair.first;
    auto delta = count_delta_pair.second;
    position = position + delta;

    // TODO: implement after implementing decode_one
    ProtocolMessage result;
    return std::make_pair(result, position);

}
}