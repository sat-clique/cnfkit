#include <cnfkit/Clause.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

using ::testing::Eq;

namespace cnfkit {

template <typename T>
class add_member {
private:
  T dummy;
};

template <typename SizeType, typename... AdditionalBases>
class alignas(8) configurable_test_clause
  : public clause<configurable_test_clause<SizeType, AdditionalBases...>, SizeType>,
    public AdditionalBases... {
public:
  using base = clause<configurable_test_clause<SizeType, AdditionalBases...>, SizeType>;
  using typename base::size_type;

  using base::begin;
  using base::cbegin;
  using base::cend;
  using base::empty;
  using base::end;
  using base::size;

  explicit configurable_test_clause(size_t size) : base(size) {}
};

template <typename ClauseType>
class ClauseTests : public ::testing::Test {
};

// clang-format off
using TestClauseTypes = ::testing::Types<
    configurable_test_clause<uint8_t>,
    configurable_test_clause<uint16_t>,
    configurable_test_clause<uint32_t>,
    configurable_test_clause<uint64_t>,

    configurable_test_clause<uint8_t, add_member<uint8_t>>,
    configurable_test_clause<uint16_t, add_member<uint8_t>>,
    configurable_test_clause<uint32_t, add_member<uint8_t>>,
    configurable_test_clause<uint64_t, add_member<uint8_t>>,

    configurable_test_clause<uint8_t, add_member<uint32_t>>,
    configurable_test_clause<uint16_t, add_member<uint32_t>>,
    configurable_test_clause<uint32_t, add_member<uint32_t>>,
    configurable_test_clause<uint64_t, add_member<uint32_t>>,

    configurable_test_clause<uint8_t, add_member<std::array<uint8_t, 5>>>,
    configurable_test_clause<uint16_t, add_member<std::array<uint8_t, 5>>>,
    configurable_test_clause<uint32_t, add_member<std::array<uint8_t, 5>>>,
    configurable_test_clause<uint64_t, add_member<std::array<uint8_t, 5>>>
  >;
// clang-format on

TYPED_TEST_SUITE(ClauseTests, TestClauseTypes);

TYPED_TEST(ClauseTests, LiteralAdressing)
{
  using test_clause = TypeParam;

  alignas(test_clause) unsigned char buf[1024];
  test_clause* clause = test_clause::base::construct_in(buf, 10);

  ASSERT_THAT(static_cast<void*>(clause), Eq(static_cast<void*>(buf)));

  uintptr_t const lits_begin_addr = reinterpret_cast<uintptr_t>(clause->begin());
  EXPECT_THAT(lits_begin_addr, Eq(reinterpret_cast<uintptr_t>(buf) + sizeof(test_clause)));
  EXPECT_THAT(lits_begin_addr % alignof(lit), Eq(0));

  uintptr_t const lits_end_addr = reinterpret_cast<uintptr_t>(clause->end());
  EXPECT_THAT(lits_end_addr - lits_begin_addr, Eq(10 * sizeof(lit)));
}

TYPED_TEST(ClauseTests, LiteralsAreZeroInitialized)
{
  using namespace cnfkit_literals;

  using test_clause = TypeParam;

  alignas(test_clause) unsigned char buf[1024];
  test_clause* clause = test_clause::base::construct_in(buf, 10);

  for (lit const& literal : *clause) {
    EXPECT_THAT(literal, Eq(-0_lit));
  }
}
}
