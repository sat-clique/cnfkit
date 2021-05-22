#pragma once

#include <cnfkit/detail/CnfLikeParser.h>

#include <filesystem>
#include <string>
#include <vector>

namespace cnfkit::detail {
template <typename BinaryFn>
void parse_drat_file_impl(std::filesystem::path const& file, BinaryFn&& clause_receiver)
{
}

template <typename BinaryFn>
void parse_drat_from_stdin_impl(BinaryFn&& clause_receiver)
{
}

template <typename BinaryFn>
void parse_drat_string_impl(std::string const& drat, BinaryFn&& clause_receiver)
{
  using namespace detail;

  cnf_chunk_parser parser;
  parser.parse(drat, 0, [&clause_receiver](std::vector<lit> const& literals) {
    clause_receiver(true, literals);
  });
}

}
