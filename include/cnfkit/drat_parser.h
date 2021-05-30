#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/detail/drat_parser.h>
#include <cnfkit/io.h>

#include <string>

/**
 * \defgroup drat_parsers DRAT Proof Parsers
 *
 * \brief Streaming parsers for DRAT proofs
 *
 * Streaming parsers for DRAT proofs expressed in the drat-trim
 * input format (see https://github.com/marijnheule/drat-trim)
 *
 * Notes:
 *  - the DRAT parser supports literals in the range of in the range `[-2^32 + 1, 2^32 - 1]`.
 *  - when parsing DRAT proofs expressed as DIMACS-like text, the parser accepts comments starting
 *    anywhere in the file.
 */

namespace cnfkit {

/**
 * Parses a source object containing a DRAT proof in text format.
 *
 * \ingroup drat_parsers
 *
 * \param source             The object to be parsed.
 * \param clause_receiver    A function with signature `void(bool, std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. The first argument is true
 *                           if and only if the clause is added to the proof.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 * \throws std::runtime_error      on I/O failure.
 */
template <typename BinaryFn>
void parse_drat_text(source& source, BinaryFn&& clause_receiver);

/**
 * Parses a source object containing a DRAT proof in binary format.
 *
 * \ingroup drat_parsers
 *
 * \param source             The object to be parsed.
 * \param clause_receiver    A function with signature `void(bool, std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. The first argument is true
 *                           if and only if the clause is added to the proof.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 * \throws std::runtime_error      on I/O failure.
 */
template <typename BinaryFn>
void parse_drat_binary(source& source, BinaryFn&& clause_receiver);


// *** Implementation ***

template <typename BinaryFn>
void parse_drat_text(source& source, BinaryFn&& clause_receiver)
{
  using namespace cnfkit::detail;

  cnf_chunk_parser parser{cnf_chunk_parser_mode::drat};
  cnf_source_reader reader{source};
  std::string buffer;
  while (!reader.is_eof()) {
    reader.read_chunk(default_chunk_size, buffer);
    parser.parse(buffer, 0, clause_receiver);
  }

  parser.check_on_drat_finish();
}

template <typename BinaryFn>
void parse_drat_binary(source& source, BinaryFn&& clause_receiver)
{
  using namespace cnfkit::detail;

  drat_binary_chunk_parser parser;
  drat_source_reader reader{source};
  while (!reader.is_eof()) {
    auto const& buffer = reader.read_chunk(default_chunk_size);
    parser.parse(buffer.data(), buffer.data() + buffer.size(), clause_receiver);
  }

  parser.check_on_drat_finish();
}
}
