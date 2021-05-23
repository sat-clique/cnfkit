#include <cnfkit/Drat.h>

#include "TestUtils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace fs = std::filesystem;

using ::testing::Eq;

namespace cnfkit {

struct parse_error {
};

using proof_clause = std::pair<bool, std::vector<lit>>;

using trivial_proof = std::vector<proof_clause>;


using DratParsingTestSpec = std::tuple<std::string,                             // description
                                       std::string,                             // input
                                       std::variant<parse_error, trivial_proof> // expected result
                                       >;

class DratParsingTests : public ::testing::TestWithParam<DratParsingTestSpec> {
public:
  auto get_input() -> std::string { return std::get<1>(GetParam()); }

  auto get_expected() -> std::variant<parse_error, trivial_proof>
  {
    return std::get<2>(GetParam());
  }
};

TEST_P(DratParsingTests, ParseFromString)
{
  DratParsingTestSpec spec = GetParam();

  if (std::holds_alternative<parse_error>(get_expected())) {
    EXPECT_THROW(
        parse_drat_string(get_input(), [](bool is_added, std::vector<lit> const& /*unused*/) {}),
        std::exception);
  }
  else {
    trivial_proof expected = std::get<trivial_proof>(get_expected());
    trivial_proof result;
    parse_drat_string(get_input(), [&result](bool is_added, std::vector<lit> const& clause) {
      result.push_back({is_added, clause});
    });
    EXPECT_THAT(result, Eq(expected));
  }
}

TEST_P(DratParsingTests, ParseFromTextFile)
{
  temp_dir const tmp{"drat_parsing_tests"};
  fs::path const cnf_file = tmp.get_path() / "proof.drat";

  {
    std::ofstream file{cnf_file};
    file << get_input();
  }

  if (std::holds_alternative<parse_error>(get_expected())) {
    EXPECT_THROW(
        parse_drat_file(cnf_file, [](bool is_added, std::vector<lit> const& /*unused*/) {}),
        std::exception);
  }
  else {
    trivial_proof expected = std::get<trivial_proof>(get_expected());
    trivial_proof result;
    parse_drat_file(cnf_file, [&result](bool is_added, std::vector<lit> const& clause) {
      result.push_back({is_added, clause});
    });
    EXPECT_THAT(result, Eq(expected));
  }
}

using namespace cnfkit_literals;

// clang-format off
INSTANTIATE_TEST_SUITE_P(DratParsingTests, DratParsingTests,
  ::testing::Values(
    std::make_tuple("parsing empty proof succeeds", "", trivial_proof{}),
    std::make_tuple("parsing proof with single illegal char fails", "x", parse_error{}),
    std::make_tuple("parsing proof with illegal char in clause fails (1)", "1 x 2 0", parse_error{}),
    std::make_tuple("parsing proof with illegal char in clause fails (2)", "1x 2 0", parse_error{}),
    std::make_tuple("parsing proof consisting of empty clause", "0", trivial_proof{proof_clause{1, {}}}),
    std::make_tuple("parsing proof ending in open clause fails (1)", "1 2 3", parse_error{}),
    std::make_tuple("parsing proof ending in open clause fails (2)", "1 2 3 0 1 2", parse_error{}),
    std::make_tuple("parsing proof containing a single unary clause", "-3 0", trivial_proof{proof_clause{1, {-3_lit}}}),
    std::make_tuple("parsing proof containing a single binary clause", "-3 1 0", trivial_proof{proof_clause{1, {-3_lit, 1_lit}}}),

    std::make_tuple("parsing proof containing a single deleted empty clause", "d 0", trivial_proof{proof_clause{0, {}}}),
    std::make_tuple("parsing proof containing a single deleted unary clause", "d -3 0", trivial_proof{proof_clause{0, {-3_lit}}}),
    std::make_tuple("parsing proof with missing space after d fails", "d-3 0", parse_error{}),
    std::make_tuple("parsing proof containing only d fails", "d", parse_error{}),
    std::make_tuple("parsing proof ending in open deleted clause fails (1)", "1 2 0 d", parse_error{}),
    std::make_tuple("parsing proof ending in deleted clause fails (2)", "1 2 0 d 1 2", parse_error{}),
    std::make_tuple("parsing proof ending in deleted clause fails (3)", "1 2 0 d\nc foo bar\n  c baz", parse_error{}),
    std::make_tuple("parsing proof ending in deleted clause fails (4)", "1 2 0 d\n", parse_error{}),

    std::make_tuple("parsing binary proof with single added empty clause",
      std::string{0x61, 0}, trivial_proof{proof_clause{true, {}}}),

    std::make_tuple("parsing binary proof with single deleted empty clause",
      std::string{0x64, 0}, trivial_proof{proof_clause{false, {}}}),

    std::make_tuple("parsing binary proof with single added unary clause (binary len 1, negative)",
      std::string{0x61, 0x7f, 0}, trivial_proof{proof_clause{true, {-63_lit}}}),

    std::make_tuple("parsing binary proof with single added unary clause (binary len 1, positive)",
      std::string{0x61, 0x02, 0}, trivial_proof{proof_clause{true, {1_lit}}}),

    std::make_tuple("parsing binary proof with single added unary clause (binary len 2, negative)",
      std::string{0x61, '\x81', 0x01, 0}, trivial_proof{proof_clause{true, {-64_lit}}}),

    std::make_tuple("parsing binary proof with single added unary clause (binary len 2, positive)",
      std::string{0x61, '\x80', 0x01, 0}, trivial_proof{proof_clause{true, {64_lit}}}),

    std::make_tuple("parsing binary proof with single added unary clause (binary len 3)",
      std::string{0x61, '\x83', '\x80', '\x01', 0}, trivial_proof{proof_clause{true, {-8193_lit}}}),

    std::make_tuple("parsing binary proof with single added unary clause (binary len 5)",
      std::string{0x61, '\x87', '\x80', '\x80', '\x80', '\x01', 0}, trivial_proof{proof_clause{true, {-134217731_lit}}}),

    std::make_tuple("parsing binary proof with two binary clauses",
      std::string{'\x64', '\x7f', '\x83', '\x80', '\x01', '\x00', '\x61', '\x82', '\x02', '\xff', '\x7f', '\x00'},
      trivial_proof{
        proof_clause{false, {-63_lit, -8193_lit}},
        proof_clause{true, {129_lit, -8191_lit}}
      }
    ),

    std::make_tuple("parsing binary proof ending in open clause fails (1)", std::string{'\x64'}, parse_error{}),
    std::make_tuple("parsing binary proof ending in open clause fails (2)", std::string{'\x64', '\x7f'}, parse_error{}),
    std::make_tuple("parsing binary proof ending in partial literal fails", std::string{'\x64', '\x83', '\x80'}, parse_error{}),
    std::make_tuple("parsing binary proof with clause ending in partial literal fails", std::string{'\x64', '\x83', '\x80', 0}, parse_error{}),
    std::make_tuple("parsing binary proof with clause not starting with a or d fails", std::string{'\x64', '\x7f', 0, '\x70', '\x7f', 0}, parse_error{}),
    std::make_tuple("parsing binary proof containing double-zero fails", std::string{'\x64', '\x7f', 0, 0, '\x61', '\x7f', 0}, parse_error{}),

    std::make_tuple("parsing binary proof containing out-of-range literal fails", std::string{'\x64', '\xff', '\xff', '\xff', '\xff', '\x7f', 0}, parse_error{})
  )
);
// clang-format on

}
