#include <cnfkit/ternary.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ostream>

using ::testing::Eq;

namespace cnfkit {

// Check regularity of tbool:
static_assert(std::is_nothrow_default_constructible_v<tbool>);
static_assert(std::is_nothrow_copy_assignable_v<tbool>);
static_assert(std::is_nothrow_copy_constructible_v<tbool>);
static_assert(std::is_nothrow_destructible_v<tbool>);
static_assert(std::is_nothrow_move_assignable_v<tbool>);
static_assert(std::is_nothrow_move_constructible_v<tbool>);
static_assert(std::is_nothrow_swappable_v<tbool>);

TEST(TernaryTests, AndSatisfiesStrongIndeterminacy)
{
  EXPECT_THAT(t_true & t_true, Eq(t_true));
  EXPECT_THAT(t_true & t_false, Eq(t_false));
  EXPECT_THAT(t_true & t_indet, Eq(t_indet));
  EXPECT_THAT(t_false & t_true, Eq(t_false));
  EXPECT_THAT(t_false & t_false, Eq(t_false));
  EXPECT_THAT(t_false & t_indet, Eq(t_false));
  EXPECT_THAT(t_indet & t_true, Eq(t_indet));
  EXPECT_THAT(t_indet & t_false, Eq(t_false));
  EXPECT_THAT(t_indet & t_indet, Eq(t_indet));
}

TEST(TernaryTests, OrSatisfiesStrongIndeterminacy)
{
  EXPECT_THAT(t_true | t_true, Eq(t_true));
  EXPECT_THAT(t_true | t_false, Eq(t_true));
  EXPECT_THAT(t_true | t_indet, Eq(t_true));
  EXPECT_THAT(t_false | t_true, Eq(t_true));
  EXPECT_THAT(t_false | t_false, Eq(t_false));
  EXPECT_THAT(t_false | t_indet, Eq(t_indet));
  EXPECT_THAT(t_indet | t_true, Eq(t_true));
  EXPECT_THAT(t_indet | t_false, Eq(t_indet));
  EXPECT_THAT(t_indet | t_indet, Eq(t_indet));
}

TEST(TernaryTests, InversionSatisfiesStrongIndeterminacy)
{
  EXPECT_THAT(!t_true, Eq(t_false));
  EXPECT_THAT(!t_false, Eq(t_true));
  EXPECT_THAT(!t_indet, Eq(t_indet));
}

TEST(TernaryTests, BoolConversion)
{
  EXPECT_THAT(to_tbool(true), Eq(t_true));
  EXPECT_THAT(to_tbool(false), Eq(t_false));
  EXPECT_THAT(static_cast<bool>(t_true), Eq(true));
  EXPECT_THAT(static_cast<bool>(t_false), Eq(false));
  EXPECT_THAT(static_cast<bool>(t_indet), Eq(false));
}
}
