#pragma once

#include <cnfkit/detail/CnfLikeParser.h>

#include <filesystem>
#include <string>
#include <vector>

namespace cnfkit::detail {
constexpr size_t default_chunk_size = (1 << 16);

template <typename BinaryFn>
auto parse_drat_gz_file(cnf_gz_file& file, BinaryFn&& clause_receiver)
{
  cnf_chunk_parser parser;

  std::string buffer;
  while (!file.is_eof()) {
    file.read_chunk(default_chunk_size, buffer);
    parser.parse(buffer, 0, [&clause_receiver](std::vector<lit> const& literals) {
      clause_receiver(true, literals);
    });
  }

  parser.check_on_drat_finish();
}

template <typename BinaryFn>
void parse_drat_file_impl(std::filesystem::path const& input_file, BinaryFn&& clause_receiver)
{
  cnf_gz_file file{input_file};
  parse_drat_gz_file(file, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_from_stdin_impl(BinaryFn&& clause_receiver)
{
  cnf_gz_file stdin_file;
  parse_drat_gz_file(stdin_file, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_string_impl(std::string const& drat, BinaryFn&& clause_receiver)
{
  cnf_chunk_parser parser;
  parser.parse(drat, 0, [&clause_receiver](std::vector<lit> const& literals) {
    clause_receiver(true, literals);
  });
  parser.check_on_drat_finish();
}
}
