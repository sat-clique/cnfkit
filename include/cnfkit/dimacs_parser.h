#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/detail/cnflike_parser.h>
#include <cnfkit/detail/dimacs_parser.h>
#include <cnfkit/io.h>
#include <cnfkit/literal.h>

#include <string>

/**
 * \defgroup dimacs_parsers DIMACS CNF Parsers
 *
 * \brief Streaming parsers for DIMACS CNF problem instances
 */

namespace cnfkit {

/**
 * \brief Parses a source object containing a DIMACS CNF problem instance.
 *
 * \ingroup dimacs_parsers
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
 *
 *
 * \param source             The object to be parsed.
 * \param clause_receiver    A function with signature `void(std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. `clause_receiver` may
 *                           throw. Exceptions thrown by `clause_receiver` are not caught by the parser.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 * \throws std::runtimer_error     on I/O failure.
 */
template <typename UnaryFn>
auto parse_cnf(source& source, UnaryFn&& clause_receiver);

// *** Implementation ***

template <typename UnaryFn>
auto parse_cnf(source& source, UnaryFn&& clause_receiver)
{
  using namespace cnfkit::detail;

  cnf_source_reader reader{source};

  std::string const header_line = reader.read_header_line();
  dimacs_problem_header header = parse_cnf_header_line(header_line);

  cnf_chunk_parser parser{cnf_chunk_parser_mode::dimacs};
  parser.parse(header_line,
               header.header_size,
               [&clause_receiver](bool /*ignored*/, std::vector<lit> const& clause) {
                 clause_receiver(clause);
               });

  std::string buffer;
  while (!reader.is_eof()) {
    reader.read_chunk(default_chunk_size, buffer);
    parser.parse(buffer, 0, [&clause_receiver](bool /*ignored*/, std::vector<lit> const& clause) {
      clause_receiver(clause);
    });
  }

  parser.check_on_dimacs_finish(header);
}
}
