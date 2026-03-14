#include <gtest/gtest.h>
#include "bin_search.h"

TEST(HelloTest, BasicAssertions) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7 * 6, 42);
}

TEST(RotatedBinSearchTest, FindsSecond) {
    std::vector<int> nums = {4, 6, 8, 10, 11, 1, 3};
    const auto it = rotated_bin_search(nums.begin(), nums.end(), 6);
    EXPECT_NE(it, nums.end());
    EXPECT_EQ(it - nums.begin(), 1);
}

TEST(RotatedBinSearchTest, FindsFirst) {
    std::vector<int> nums = {3, 4, 6, 8, 10, 11, 1};
    const auto it = rotated_bin_search(nums.begin(), nums.end(), 3);
    EXPECT_NE(it, nums.end());
    EXPECT_EQ(it - nums.begin(), 0);
}

TEST(RotatedBinSearchTest, FindsLast) {
    std::vector<int> nums = {3, 4, 6, 8, 10, 11, 1};
    const auto it = rotated_bin_search(nums.begin(), nums.end(), 1);
    EXPECT_NE(it, nums.end());
    EXPECT_EQ(it - nums.begin(), 6);
}

TEST(RotatedBinSearchTest, FindsUnrotated) {
    std::vector<int> nums = {1, 3, 4, 6, 8, 10, 11};
    const auto it = rotated_bin_search(nums.begin(), nums.end(), 10);
    EXPECT_NE(it, nums.end());
    EXPECT_EQ(it - nums.begin(), 5);
}

TEST(RotatedBinSearchTest, RightSorted) {
    std::vector<int> nums = {10, 11, 1, 3, 4, 6, 8};
    const auto it = rotated_bin_search(nums.begin(), nums.end(), 11);
    EXPECT_NE(it, nums.end());
    EXPECT_EQ(it - nums.begin(), 1);
}