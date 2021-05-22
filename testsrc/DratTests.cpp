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
    std::make_tuple("parsing proof consisting of empty clause succeeds", "0", trivial_proof{proof_clause{1, {}}}),
    std::make_tuple("parsing proof ending in open clause fails (1)", "1 2 3", parse_error{}),
    std::make_tuple("parsing proof ending in open clause fails (2)", "1 2 3 0 1 2", parse_error{})
  )
);
// clang-format on

}
