#include <cnfkit/Dimacs.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ostream>
#include <string>
#include <variant>
#include <vector>

using ::testing::Eq;

namespace cnfkit {

struct parse_error {
};

using trivial_formula = std::vector<std::vector<lit>>;


using DimacsParsingTestSpec =
    std::tuple<std::string,                               // description
               std::string,                               // input
               std::variant<parse_error, trivial_formula> // expected result
               >;

class DimacsParsingTests : public ::testing::TestWithParam<DimacsParsingTestSpec> {
public:
  auto get_input() -> std::string { return std::get<1>(GetParam()); }

  auto get_expected() -> std::variant<parse_error, trivial_formula>
  {
    return std::get<2>(GetParam());
  }
};

std::ostream& operator<<(std::ostream& stream, lit it)
{
  stream << (it.is_positive() ? 1 : -1) * static_cast<int64_t>(it.get_var().get_raw_value());
  return stream;
}

TEST_P(DimacsParsingTests, ParseFromString)
{
  DimacsParsingTestSpec spec = GetParam();

  if (std::holds_alternative<parse_error>(get_expected())) {
    EXPECT_THROW(parse_cnf_string(get_input(), [](std::vector<lit> const& /*unused*/) {}),
                 std::exception);
  }
  else {
    trivial_formula expected = std::get<trivial_formula>(get_expected());
    trivial_formula result;
    parse_cnf_string(get_input(),
                     [&result](std::vector<lit> const& clause) { result.push_back(clause); });
    EXPECT_THAT(result, Eq(expected));
  }
}

using namespace cnfkit_literals;

// clang-format off
INSTANTIATE_TEST_SUITE_P(DimacsParsingTests, DimacsParsingTests,
  ::testing::Values(
    std::make_tuple("parsing empty string fails", "", parse_error{}),
    std::make_tuple("parsing illegal header 'x cnf' fails", "x cnf 0 0", parse_error{}),
    std::make_tuple("parsing too short header 'p cnf 0' fails", "p cnf 0", parse_error{}),
    std::make_tuple("parsing header with negative number of variables fails", "p cnf -1 0", parse_error{}),
    std::make_tuple("parsing header with negative number of clauses fails", "p cnf 0 -1", parse_error{}),

    std::make_tuple("parsing empty problem", "p cnf 0 0", trivial_formula{}),
    std::make_tuple("parsing problem with single empty clause", "p cnf 0 1 0", trivial_formula{{}}),
    std::make_tuple("parsing problem with single unary clause", "p cnf 1 1 -5 0", trivial_formula{{-5_lit}}),

    std::make_tuple("parsing non-integer variables fails", "p cnf 1 1 x 0", parse_error{}),

    std::make_tuple("parsing problem with two clauses",
      "p cnf 3 2 -1 2 0 2 -3 1 0",
      trivial_formula{{-1_lit, 2_lit}, {2_lit, -3_lit, 1_lit}}),

    std::make_tuple("parsing problem with comment line",
      "p cnf 2 1\nc foo\n1 2 0",
      trivial_formula{{1_lit, 2_lit}}),

    std::make_tuple("parsing problem with comment line, whitespace before c",
      "p cnf 2 1\n\r\t c foo\n1 2 0",
      trivial_formula{{1_lit, 2_lit}}),

    std::make_tuple("parsing problem with whitespace at start of line",
        "p cnf 1 1 \t\v\r\n -5 0",
        trivial_formula{{-5_lit}}),

    std::make_tuple("parsing problem with comment in clause",
        "p cnf 2 1\n1\nc foo\n2 0",
        trivial_formula{{1_lit, 2_lit}}),

    std::make_tuple("parsing problem with whitespaces and comments",
      "p cnf 10 5\nc \v bar\n\t\t2 3\r\n\r4 0     0 -1 0\t1\r2\n3\v0 1\nc baz   \n -2 0",
      trivial_formula{{2_lit, 3_lit, 4_lit}, {}, {-1_lit}, {1_lit, 2_lit, 3_lit}, {1_lit, -2_lit}}),

    std::make_tuple("parsing problem with header preceded by comment",
      "c foo\np cnf 1 1\n1 0",
      trivial_formula{{1_lit}}),

    std::make_tuple("parsing problem with header preceded by multiple comments",
      "c foo\n\nc bar\np cnf 1 1\n1 0",
      trivial_formula{{1_lit}}),

    std::make_tuple("parsing problem ending with comments",
      "p cnf 1 1\n1 0\nc foo\nc bar",
      trivial_formula{{1_lit}}),

    std::make_tuple("parsing problem not ending with 0 fails", "p cnf 1 1 -5 0 -1", parse_error{}),

    std::make_tuple("parsing problem containing maximum variable",
      "p cnf 1 1 " + std::to_string(max_raw_var) + " -" + std::to_string(max_raw_var) + " 0",
      trivial_formula{{lit{var{max_raw_var}, true}, lit{var{max_raw_var}, false}}}),

    std::make_tuple("parsing problem containing variable > max variable (positive) fails",
      "p cnf 1 1 " + std::to_string(max_raw_var + 1) + " 0",
      parse_error{}),

    std::make_tuple("parsing problem containing variable > max variable (negative) fails",
      "p cnf 1 1 -" + std::to_string(max_raw_var + 1) + " 0",
      parse_error{}),

    std::make_tuple("parsing problem containing variable > max variable (min int32) fails",
      "p cnf 1 1 " + std::to_string(std::numeric_limits<int>::min()) + " 0",
      parse_error{}),

    std::make_tuple("parsing problem containing variable > max variable (min int32 - 1) fails",
      "p cnf 1 1 " + std::to_string(static_cast<int64_t>(std::numeric_limits<int>::min()) - 1) + " 0",
      parse_error{}),

    std::make_tuple("parsing problem containing variable > max variable (max int32 + 1) fails",
      "p cnf 1 1 " + std::to_string(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1) + " 0",
      parse_error{})
  )
);
// clang-format on

}
