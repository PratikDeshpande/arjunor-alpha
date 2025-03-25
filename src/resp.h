#include <stddef.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <sstream>

// TODO: Find a common response interface/type that represents a union of all types returned by read_* functions
//  so that byte streams can be processed in a chain / (ie composition)
//  Possible options: response struct, union struct, etc
namespace resp {

    enum ProtocolMessageType { SimpleString, SimpleError, Integer, BulkString, Array, Null, Boolean, Double, BigNumber, BulkError, VerbatimString, Map, Attribute, Set, Push };
    struct ProtocolMessage {
        ProtocolMessageType protocol_message_type;
        std::variant<std::string, int64_t, std::vector<std::shared_ptr<ProtocolMessage>>> data;
    };

    std::pair<int, int> read_length(const std::vector<std::byte>& buffer);

    std::pair<std::string, int> read_simple_string(const std::vector<std::byte>& buffer);

    std::pair<std::string, int> read_error(const std::vector<std::byte>& buffer);

    std::pair<std::int64_t, int> read_int_64(const std::vector<std::byte>& buffer);

    std::pair<std::string, int> read_bulk_string(const std::vector<std::byte>& buffer);

    std::pair<std::shared_ptr<ProtocolMessage>, int> read_array(const std::vector<std::byte>& buffer);

    std::pair<std::shared_ptr<ProtocolMessage>, int> decode_one(const std::vector<std::byte>& buffer);

    std::pair<std::shared_ptr<ProtocolMessage>, int> decode(const std::vector<std::byte>& buffer);

    // Encode functions
    std::vector<std::byte> encode_simple_string(const std::string& data);
    std::vector<std::byte> encode_bulk_string(const std::string& data);
    std::vector<std::byte> encode_null();









}