#include <cnfkit/DratWriter.h>

#include <cnfkit/DratParser.h>
#include <cnfkit/Literal.h>
#include <cnfkit/Sink.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using ::testing::Eq;

namespace cnfkit {

namespace {
class test_sink : public sink {
public:
  void write_bytes(std::byte const* start, std::byte const* stop) override
  {
    m_buffer.insert(m_buffer.end(), start, stop);
  }

  void flush() override {}

  auto as_string() const -> std::string
  {
    std::string result;
    for (std::byte byte : m_buffer) {
      result.push_back(std::to_integer<char>(byte));
    }
    return result;
  }

  auto bytes() -> std::vector<std::byte> const& { return m_buffer; }

private:
  std::vector<std::byte> m_buffer;
};

using test_proof_clause = std::pair<bool, std::vector<lit>>;
using test_proof = std::vector<test_proof_clause>;
using drat_writer_test_input = std::tuple<std::string, test_proof>;
}

class DratWriterTest : public ::testing::TestWithParam<drat_writer_test_input> {
protected:
  auto get_input() const -> test_proof { return std::get<1>(GetParam()); }
};

namespace {
void send_test_input_to_writer(test_proof const& test_proof, drat_writer& target)
{
  for (test_proof_clause const& clause : test_proof) {
    auto const& lits = clause.second;
    if (clause.first) {
      target.add_clause(lits.data(), lits.data() + lits.size());
    }
    else {
      target.del_clause(lits.data(), lits.data() + lits.size());
    }
  }
}

struct test_drat_proof_collector {
  void operator()(bool is_add, std::vector<lit> lits)
  {
    result.push_back(test_proof_clause{is_add, lits});
  }

  test_proof result;
};

}

TEST_P(DratWriterTest, WriteAsString)
{
  test_sink sink;
  drat_writer under_test{sink, drat_format::text};
  send_test_input_to_writer(get_input(), under_test);

  under_test.flush();

  std::string const result = sink.as_string();
  test_drat_proof_collector collector;
  parse_drat_string(result, collector);

  EXPECT_THAT(collector.result, Eq(get_input()));
}

TEST_P(DratWriterTest, WriteAsBinary)
{
  test_sink sink;
  drat_writer under_test{sink, drat_format::binary};
  send_test_input_to_writer(get_input(), under_test);

  under_test.flush();

  auto const& result = sink.bytes();
  test_drat_proof_collector collector;
  parse_drat_binary_buffer(result.data(), result.data() + result.size(), collector);

  EXPECT_THAT(collector.result, Eq(get_input()));
}

using namespace cnfkit_literals;

// clang-format off
INSTANTIATE_TEST_SUITE_P(DratWriterTest, DratWriterTest,
  ::testing::Values(
    std::make_tuple("empty proof", test_proof{}),
    std::make_tuple("writing proof with single empty clause", test_proof{{true, {}}}),
    std::make_tuple("writing proof with single unary added clause",
                    test_proof{{true, {-3_dlit}}}),
    std::make_tuple("writing proof with single unary added clause",
                    test_proof{{false, {-3_dlit}}}),
    std::make_tuple("writing proof with single binary added clause",
                    test_proof{{true, {-3_dlit, 1024_dlit}}}),
    std::make_tuple("writing proof with single binary added clause",
                    test_proof{{false, {-3_dlit, 1024_dlit}}}),

    std::make_tuple("writing proof with multiple clauses",
                    test_proof{{false, {1_dlit, -2_dlit, 9_dlit}},
                               {true, {10000_dlit, 7_dlit}},
                               {false, {-2000000000_dlit}},
                               {false, {10_dlit, 11_dlit, 12_dlit, 13_dlit, 14_dlit}}}),

    std::make_tuple("writing proof containing maximal literals",
                    test_proof{{true, {dimacs_to_lit(min_dimacs_lit), dimacs_to_lit(max_dimacs_lit)}}})));
// clang-format on
}
