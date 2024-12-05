#include <stddef.h>
#include <sys/types.h>
#include <string>
#include <vector>

// TODO: Find a common response interface/type that represents a union of all types returned by read_* functions
//  so that byte streams can be processed in a chain / (ie composition)
//  Possible options: response struct, union struct, etc
namespace response {

    enum ProtocolMessageType { SimpleString, SimpleError, Integer, BulkString, Array, Null, Boolean, Double, BigNumber, BulkError, VerbatimString, Map, Attribute, Set, Push };
    struct ProtocolMessage {
        ProtocolMessageType protocol_message_type;
        std::string string_value; //TODO: char* or std::string or std::string_view ?
        int64_t int_value;
        ProtocolMessage* elements;
    };

    std::pair<int, int> read_length(const std::vector<std::byte>& buffer);

    std::pair<std::string, int> read_simple_string(const std::vector<std::byte>& buffer);

    std::pair<std::string, int> read_error(const std::vector<std::byte>& buffer);

    std::pair<std::int64_t, int> read_int_64(const std::vector<std::byte>& buffer);

    std::pair<std::string, int> read_bulk_string(const std::vector<std::byte>& buffer);

    std::pair<ProtocolMessage, int> read_array(const std::vector<std::byte>& buffer);







}