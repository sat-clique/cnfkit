#pragma once

#include <cnfkit/detail/CheckCxxVersion.h>

#include <cnfkit/Literal.h>
#include <cnfkit/detail/CnfLikeParser.h>
#include <cnfkit/detail/DimacsParser.h>

#include <filesystem>
#include <string>


namespace cnfkit {
template <typename UnaryFn>
void parse_cnf_file(std::filesystem::path const& input_file, UnaryFn&& clause_receiver);

template <typename UnaryFn>
void parse_cnf_from_stdin(UnaryFn&& clause_receiver);

template <typename UnaryFn>
void parse_cnf_string(std::string const& cnf, UnaryFn&& clause_receiver);


// *** Implementation ***

template <typename UnaryFn>
void parse_cnf_file(std::filesystem::path const& input_file, UnaryFn&& clause_receiver)
{
  using namespace detail;
  cnf_gz_file file{input_file};
  parse_cnf_gz_file(file, clause_receiver);
}

template <typename UnaryFn>
void parse_cnf_from_stdin(UnaryFn&& clause_receiver)
{
  using namespace detail;
  cnf_gz_file stdin_file;
  parse_cnf_gz_file(stdin_file, clause_receiver);
}

template <typename UnaryFn>
void parse_cnf_string(std::string const& cnf, UnaryFn&& clause_receiver)
{
  using namespace detail;

  dimacs_problem_header header = parse_cnf_header_line(cnf);
  cnf_chunk_parser parser{cnf_chunk_parser_mode::dimacs};
  parser.parse(cnf,
               header.header_size,
               [&clause_receiver](bool /*ignored*/, std::vector<lit> const& clause) {
                 clause_receiver(clause);
               });
  parser.check_on_dimacs_finish(header);
}
}
