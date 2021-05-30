#pragma once

#include <cnfkit/detail/cnflike_parser.h>
#include <cnfkit/literal.h>

#include <charconv>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cnfkit::detail {

inline auto parse_cnf_header_line(std::string_view buffer) -> dimacs_problem_header
{
  auto const [buffer_past_comments, ended_in_comment] =
      skip_dimacs_comments(buffer.begin(), buffer.end());
  // ignoring ended_in_comment since it can only be true if
  // buffer_past_comments == buffer.end(), causing parse failure

  std::regex const header_regex{"^\\s*p\\s*cnf\\s*([0-9]+)\\s*([0-9]+)"};
  std::match_results<std::string_view::const_iterator> header_match;
  if (std::regex_search(buffer_past_comments, buffer.end(), header_match, header_regex)) {
    if (header_match.size() != 3) {
      throw std::invalid_argument{"Syntax error in CNF header"};
    }

    dimacs_problem_header result;

    result.header_size = header_match[0].second - header_match[0].first +
                         std::distance(buffer.begin(), buffer_past_comments);

    auto const [next, ec] =
        std::from_chars(header_match[1].first, header_match[1].second, result.num_vars);
    if (ec != std::errc{}) {
      throw std::invalid_argument{"Invalid number of variables"};
    }

    auto const [next2, ec2] =
        std::from_chars(header_match[2].first, header_match[2].second, result.num_clauses);
    if (ec2 != std::errc{}) {
      throw std::invalid_argument{"Invalid number of clauses"};
    }

    return result;
  }
  else {
    throw std::invalid_argument{"Syntax error in CNF header"};
  }
}
}
