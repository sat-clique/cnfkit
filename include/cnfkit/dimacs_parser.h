#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/detail/cnflike_parser.h>
#include <cnfkit/detail/dimacs_parser.h>
#include <cnfkit/literal.h>

#include <cnfkit/io/io_buf.h>
#include <cnfkit/io/io_zlib.h>

#include <filesystem>
#include <string>

/**
 * \defgroup dimacs_parsers DIMACS CNF Parsers
 *
 * \brief Streaming parsers for DIMACS CNF problem instances
 *
 * The DIMACS CNF format is described in
 * https://www.cs.utexas.edu/users/moore/acl2/manuals/current/manual/index-seo.php/SATLINK____DIMACS
 *
 * Notes:
 *  - the CNF parser supports literals in the range `[-2^32 + 1, 2^32 - 1]`.
 *  - since CNF writers don't handle the variable-count field in the DIMACS
 *    header consistently (number of distinct variables vs. maximum occurring
 *    variable), the value of the variable-count field is ignored by the CNF
 *    parser, though it requires the value to be a 32-bit signed integer.
 *  - the parser does not require comments to start at the beginning of lines,
 *    though they may not start within the DIMACS header.
 */

namespace cnfkit {

/**
 * Parses a file containing a DIMACS CNF problem instance.
 *
 * \ingroup dimacs_parsers
 *
 * \param file               Path of the file to be parsed.
 * \param clause_receiver    A function with signature `void(std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. `clause_receiver` may
 *                           throw. Exceptions thrown by `clause_receiver` are not caught by the parser.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename UnaryFn>
void parse_cnf_file(std::filesystem::path const& input_file, UnaryFn&& clause_receiver);

/**
 * Parses a DIMACS CNF problem instance from stdin.
 *
 * \ingroup dimacs_parsers
 *
 * \param clause_receiver    A function with signature `void(std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. `clause_receiver` may
 *                           throw. Exceptions thrown by `clause_receiver` are not caught by the parser.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename UnaryFn>
void parse_cnf_from_stdin(UnaryFn&& clause_receiver);

/**
 * Parses a string containing a DIMACS CNF problem instance.
 *
 * \ingroup dimacs_parsers
 *
 * \param cnf                The string to be parsed.
 * \param clause_receiver    A function with signature `void(std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. `clause_receiver` may
 *                           throw. Exceptions thrown by `clause_receiver` are not caught by the parser.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename UnaryFn>
void parse_cnf_string(std::string const& cnf, UnaryFn&& clause_receiver);


// *** Implementation ***

template <typename UnaryFn>
void parse_cnf_file(std::filesystem::path const& input_file, UnaryFn&& clause_receiver)
{
  using namespace detail;
  zlib_source source{input_file};
  parse_cnf_source(source, clause_receiver);
}

template <typename UnaryFn>
void parse_cnf_from_stdin(UnaryFn&& clause_receiver)
{
  using namespace detail;
  zlib_source source;
  parse_cnf_source(source, clause_receiver);
}

template <typename UnaryFn>
void parse_cnf_string(std::string const& cnf, UnaryFn&& clause_receiver)
{
  using namespace detail;
  buf_source source{cnf};
  parse_cnf_source(source, clause_receiver);
}
}
