
#include "../src/resp.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <tuple>
#include <memory>
// TODO: Regarding includes: Is best practice to remove redundant imports or to have all needed imports in file? (if project org changes, then the dep chain may break)


TEST(DecodeTest, TestReadSimpleString) {
    auto char_string = "+OK\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 5);

    auto result = resp::read_simple_string(byte_vector);
    EXPECT_EQ(result.first, "OK");
    EXPECT_EQ(result.second, 5);
}

TEST(DecodeTest, TestError) {
    auto char_string = "-Error message\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 16);

    auto result = resp::read_simple_string(byte_vector);
    EXPECT_EQ(result.first, "Error message");
    EXPECT_EQ(result.second, 16);
}

TEST(DecodeTest, TestInt64) {
    std::array<std::tuple<std::string, int64_t, int>, 2> test_cases = {
        std::make_tuple(":0\r\n", 0, 4),
        std::make_tuple(":1000\r\n", 1000, 7),
    };

    for (auto test_case : test_cases) {
        auto char_string = std::get<0>(test_case).c_str();
        auto byte_string = (std::byte*)char_string;
        std::vector<std::byte> byte_vector(byte_string, byte_string + std::get<2>(test_case));

        auto result = resp::read_int_64(byte_vector);
        EXPECT_EQ(result.first, std::get<1>(test_case));
        EXPECT_EQ(result.second, std::get<2>(test_case));
    }
}

TEST(DecodeTest, TestReadBulkString) {
    // TODO: Add case for null string $-1\r\n
    std::array<std::tuple<std::string, std::string, int>, 2> test_cases = {
        std::make_tuple("$5\r\nhello\r\n", "hello", 11),
        std::make_tuple("$0\r\n\r\n", "", 6),
        //std::make_tuple("$-1\r\n", "", 4)
    };

    for (auto test_case : test_cases) {
        auto char_string = std::get<0>(test_case).c_str();
        auto byte_string = (std::byte*)char_string;
        std::vector<std::byte> byte_vector(byte_string, byte_string + std::get<2>(test_case));

        auto result = resp::read_bulk_string(byte_vector);
        EXPECT_EQ(result.first, std::get<1>(test_case));
        EXPECT_EQ(result.second, std::get<2>(test_case));
    }
}

TEST(DecodeTest, TestReadArrayEmpty) {
    auto char_string = "*0\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 4);

    auto result = resp::read_array(byte_vector);
    auto protocol_message = result.first;
    auto data = protocol_message->data;
    auto vectors = std::get<std::vector<std::shared_ptr<resp::ProtocolMessage>>>(data);
    for (auto vec: vectors) {
        if (vec->protocol_message_type == resp::ProtocolMessageType::BulkString) {
            auto vec_result =  std::get<std::string>(vec->data);
            std::cout << "bulk string result: " << vec_result << std::endl;
        }
    }
    EXPECT_EQ(protocol_message->protocol_message_type, resp::ProtocolMessageType::Array);
    EXPECT_EQ(vectors.size(), 0);
}

// TODO: Add cases for array of ints, and array of different types
TEST(DecodeTest, TestReadArray) {
    auto char_string = "*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 26);

    auto result = resp::read_array(byte_vector);
    auto protocol_message = result.first;
    auto data = protocol_message->data;
    auto vectors = std::get<std::vector<std::shared_ptr<resp::ProtocolMessage>>>(data);
    EXPECT_EQ(protocol_message->protocol_message_type, resp::ProtocolMessageType::Array);
    EXPECT_EQ(vectors.size(), 2);
    EXPECT_EQ(vectors[0]->protocol_message_type, resp::ProtocolMessageType::BulkString);
    EXPECT_EQ(vectors[1]->protocol_message_type, resp::ProtocolMessageType::BulkString);
    EXPECT_EQ(std::get<std::string>(vectors[0]->data), "hello");
    EXPECT_EQ(std::get<std::string>(vectors[1]->data), "world");
}

TEST(DecodeTest, TestReadArrayMixed) {
    auto char_string = "*3\r\n$1\r\na\r\n:200\r\n$3\r\ncat\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 26);

    auto result = resp::read_array(byte_vector);
    auto protocol_message = result.first;
    auto data = protocol_message->data;
    auto vectors = std::get<std::vector<std::shared_ptr<resp::ProtocolMessage>>>(data);
    EXPECT_EQ(protocol_message->protocol_message_type, resp::ProtocolMessageType::Array);
    EXPECT_EQ(vectors.size(), 3);
    EXPECT_EQ(vectors[0]->protocol_message_type, resp::ProtocolMessageType::BulkString);
    EXPECT_EQ(vectors[1]->protocol_message_type, resp::ProtocolMessageType::Integer);
    EXPECT_EQ(vectors[2]->protocol_message_type, resp::ProtocolMessageType::BulkString);
    EXPECT_EQ(std::get<std::string>(vectors[0]->data), "a");
    EXPECT_EQ(std::get<int64_t>(vectors[1]->data), 200);
    EXPECT_EQ(std::get<std::string>(vectors[2]->data), "cat");
}

TEST(DecodeTest, TestReadLength) {
    auto char_string = "24\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 4);

    auto result = resp::read_length(byte_vector);
    EXPECT_EQ(result.first, 24);
    EXPECT_EQ(result.second, 4);
}

TEST(DecodeTest, TestDecodeOne) {
    // TODO: Automate logic for converting string to byte vector (not straightforward to count size with escape characters)
    // TODO: add edge cases for empty elements
    std::array<std::tuple<std::string, int, resp::ProtocolMessageType>, 5> test_cases = {
        std::make_tuple("+OK\r\n", 5, resp::ProtocolMessageType::SimpleString),
        std::make_tuple("-Error message\r\n", 16, resp::ProtocolMessageType::SimpleError),
        std::make_tuple(":1000\r\n", 7, resp::ProtocolMessageType::Integer),
        std::make_tuple("$5\r\nhello\r\n", 11, resp::ProtocolMessageType::BulkString),
        std::make_tuple("*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n", 26, resp::ProtocolMessageType::Array) 
    };

    for (auto test_case : test_cases) {
        auto char_string = std::get<0>(test_case).c_str();
        auto byte_string = (std::byte*)char_string;
        std::vector<std::byte> byte_vector(byte_string, byte_string + std::get<1>(test_case));

        auto result = resp::decode_one(byte_vector);
        EXPECT_EQ(result.first->protocol_message_type, std::get<2>(test_case));
    }
}


TEST(DecodeTest, TestEmptyBuffer) {
    const std::vector<std::byte> empty_buffer;
    EXPECT_THROW(resp::decode(empty_buffer), std::invalid_argument);
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

