
#include "../src/response.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <tuple>

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

TEST(DecodeTest, TestReadLength) {
    auto char_string = "24\r\n";
    auto byte_string = (std::byte*)char_string;
    std::vector<std::byte> byte_vector(byte_string, byte_string + 4);

    auto result = response::read_length(byte_vector);
    EXPECT_EQ(result.first, 24);
    EXPECT_EQ(result.second, 4);
}






int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

