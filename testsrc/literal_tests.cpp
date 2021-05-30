#include <cnfkit/literal.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

using ::testing::Eq;

namespace cnfkit {

// Check whether lit and var are regular types (+ noexcept):
static_assert(std::is_nothrow_default_constructible_v<var>);
static_assert(std::is_nothrow_copy_assignable_v<var>);
static_assert(std::is_nothrow_copy_constructible_v<var>);
static_assert(std::is_nothrow_destructible_v<var>);
static_assert(std::is_nothrow_move_assignable_v<var>);
static_assert(std::is_nothrow_move_constructible_v<var>);
static_assert(std::is_nothrow_swappable_v<var>);

static_assert(std::is_nothrow_default_constructible_v<lit>);
static_assert(std::is_nothrow_copy_assignable_v<lit>);
static_assert(std::is_nothrow_copy_constructible_v<lit>);
static_assert(std::is_nothrow_destructible_v<lit>);
static_assert(std::is_nothrow_move_assignable_v<lit>);
static_assert(std::is_nothrow_move_constructible_v<lit>);
static_assert(std::is_nothrow_swappable_v<lit>);

using namespace cnfkit_literals;

TEST(LiteralTests, VariableSuccessors)
{
  EXPECT_THAT(var{0}.next(), Eq(var{1}));
  EXPECT_THAT(var{1}.prev(), Eq(var{0}));
}

TEST(LiteralTests, LiteralSuccessors)
{
  EXPECT_THAT((1_lit).next(), Eq(-2_lit));
  EXPECT_THAT((-2_lit).next(), 2_lit);
  EXPECT_THAT((1_lit).prev(), -1_lit);
  EXPECT_THAT((-1_lit).prev(), 0_lit);

  EXPECT_THAT((2_lit).next_with_same_sign(), 3_lit);
  EXPECT_THAT((-2_lit).next_with_same_sign(), -3_lit);
  EXPECT_THAT((2_lit).prev_with_same_sign(), 1_lit);
  EXPECT_THAT((-2_lit).prev_with_same_sign(), -1_lit);
}

TEST(LiteralTests, VariableIncrement)
{
  var under_test = 2_var;
  EXPECT_THAT(++under_test, Eq(3_var));
  EXPECT_THAT(under_test++, Eq(3_var));
  EXPECT_THAT(under_test, Eq(4_var));
}

TEST(LiteralTests, VariableDecrement)
{
  var under_test = 4_var;
  EXPECT_THAT(--under_test, Eq(3_var));
  EXPECT_THAT(under_test--, Eq(3_var));
  EXPECT_THAT(under_test, Eq(2_var));
}

TEST(LiteralTests, LiteralIncrement)
{
  lit under_test = 2_lit;
  EXPECT_THAT(++under_test, Eq(-3_lit));
  EXPECT_THAT(under_test++, Eq(-3_lit));
  EXPECT_THAT(under_test, Eq(3_lit));
}

TEST(LiteralTests, LiteralDecrement)
{
  lit under_test = 3_lit;
  EXPECT_THAT(--under_test, Eq(-3_lit));
  EXPECT_THAT(under_test--, Eq(-3_lit));
  EXPECT_THAT(under_test, Eq(2_lit));
}

TEST(LiteralTests, DimacsToLit)
{
  EXPECT_THROW(dimacs_to_lit(0), std::invalid_argument);
  EXPECT_THAT(dimacs_to_lit(1), Eq(1_dlit));
  EXPECT_THAT(dimacs_to_lit(-1), Eq(-1_dlit));

  var const max_dimacs_var = var{max_var.get_raw_value() - 1};
  EXPECT_THAT(dimacs_to_lit(max_dimacs_lit), Eq(lit{max_dimacs_var, true}));
  EXPECT_THAT(dimacs_to_lit(min_dimacs_lit), Eq(lit{max_dimacs_var, false}));
}

TEST(LiteralTests, LitToDimacs)
{
  EXPECT_THAT(lit_to_dimacs(1_dlit), Eq(1));
  EXPECT_THAT(lit_to_dimacs(-1_dlit), Eq(-1));

  var const max_dimacs_var = var{max_var.get_raw_value() - 1};
  EXPECT_THAT(lit_to_dimacs(lit{max_dimacs_var, true}), Eq(max_dimacs_lit));
  EXPECT_THAT(lit_to_dimacs(lit{max_dimacs_var, false}), Eq(min_dimacs_lit));

  EXPECT_THROW(lit_to_dimacs(lit{max_var, true}), std::invalid_argument);
  EXPECT_THROW(lit_to_dimacs(lit{max_var, false}), std::invalid_argument);
}

}
