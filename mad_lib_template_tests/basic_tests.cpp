#include "gtest/gtest.h"
#include "mad_lib_template.hpp"
#include <vector>
#include <algorithm>

TEST(Add, VeryBasic)
{
  size_t res = mad_lib_template::Add(1, 1);
  ASSERT_EQ(res, 2) << "DO YOU EVEN MATH?";
}

TEST(Add, Basic)
{
  std::vector<int> Test(100);
  std::generate(Test.begin(), Test.end(), std::rand);
  for (auto const &a : Test)
  {
    size_t res = mad_lib_template::Add(a, a);
    ASSERT_EQ(res, a + a) << "DO YOU EVEN MATH?";
  }
}

TEST(Mult, Basic)
{
  std::vector<int> Test(100);
  std::generate(Test.begin(), Test.end(), std::rand);
  for (auto const &a : Test)
  {
    size_t res = mad_lib_template::Mult(a, a);
    ASSERT_EQ(res, a * a) << "DO YOU EVEN MATH?";
  }
}

// TEST_SUITE
class AddTest : public ::testing::TestWithParam<int>
{
};

TEST_P(AddTest, Basic)
{
  // Inside a test, access the test parameter with the GetParam() method
  // of the TestWithParam<T> class:
  ASSERT_EQ(mad_lib_template::Add(GetParam(), GetParam()), GetParam() + GetParam());
}

INSTANTIATE_TEST_SUITE_P(AddInstance,
                         AddTest,
                         testing::Values(1, 2, 3));