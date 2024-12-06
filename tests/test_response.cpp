
#include "../src/response.h"
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

    auto result = response::read_simple_string(byte_vector);
    EXPECT_EQ(result.first, "OK");
    EXPECT_EQ(result.second, 5);
}

TEST(DecodeTest, TestError) {
    auto char_string = "-Error message\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 16);

    auto result = response::read_simple_string(byte_vector);
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

        auto result = response::read_int_64(byte_vector);
        EXPECT_EQ(result.first, std::get<1>(test_case));
        EXPECT_EQ(result.second, std::get<2>(test_case));
    }
}

TEST(DecodeTest, TestReadBulkString) {
    auto char_string = "$5\r\nhello\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 11);

    auto result = response::read_bulk_string(byte_vector);
    EXPECT_EQ(result.first, "hello");
    EXPECT_EQ(result.second, 11);
}

TEST(DecodeTest, TestReadArrayEmpty) {
    auto char_string = "*0\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 4);

    auto result = response::read_array(byte_vector);
    auto protocol_message = result.first;
    auto data = protocol_message->data;
    auto vectors = std::get<std::vector<std::shared_ptr<response::ProtocolMessage>>>(data);
    for (auto vec: vectors) {
        if (vec->protocol_message_type == response::ProtocolMessageType::BulkString) {
            auto vec_result =  std::get<std::string>(vec->data);
            std::cout << "bulk string result: " << vec_result << std::endl;
        }
    }
    EXPECT_EQ(protocol_message->protocol_message_type, response::ProtocolMessageType::Array);
    EXPECT_EQ(vectors.size(), 0);
}

// TODO: Add cases for array of ints, and array of different types
TEST(DecodeTest, TestReadArray) {
    auto char_string = "*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 26);

    auto result = response::read_array(byte_vector);
    auto protocol_message = result.first;
    auto data = protocol_message->data;
    auto vectors = std::get<std::vector<std::shared_ptr<response::ProtocolMessage>>>(data);
    EXPECT_EQ(protocol_message->protocol_message_type, response::ProtocolMessageType::Array);
    EXPECT_EQ(vectors.size(), 2);
    // TODO: Test for containment for strings "hello" and "world"
}

TEST(DecodeTest, TestReadLength) {
    auto char_string = "24\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 4);

    auto result = response::read_length(byte_vector);
    EXPECT_EQ(result.first, 24);
    EXPECT_EQ(result.second, 4);
}

TEST(DecodeTest, TestDecodeOne) {
    // TODO: Automate logic for converting string to byte vector (not straightforward to count size with escape characters)
    // TODO: add edge cases for empty elements
    std::array<std::tuple<std::string, int, response::ProtocolMessageType>, 5> test_cases = {
        std::make_tuple("+OK\r\n", 5, response::ProtocolMessageType::SimpleString),
        std::make_tuple("-Error message\r\n", 16, response::ProtocolMessageType::SimpleError),
        std::make_tuple(":1000\r\n", 7, response::ProtocolMessageType::Integer),
        std::make_tuple("$5\r\nhello\r\n", 11, response::ProtocolMessageType::BulkString),
        std::make_tuple("*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n", 26, response::ProtocolMessageType::Array) 
    };

    for (auto test_case : test_cases) {
        auto char_string = std::get<0>(test_case).c_str();
        auto byte_string = (std::byte*)char_string;
        std::vector<std::byte> byte_vector(byte_string, byte_string + std::get<1>(test_case));

        auto result = response::decode_one(byte_vector);
        EXPECT_EQ(result.first->protocol_message_type, std::get<2>(test_case));
    }
}





int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

