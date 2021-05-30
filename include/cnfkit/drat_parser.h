#pragma once

/**
 * \file
 */

#include <cnfkit/detail/check_cxx_version.h>

#include <cnfkit/literal.h>
#include <cnfkit/detail/drat_parser.h>

#include <cstddef>
#include <filesystem>
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
 * Parses a file containing a DRAT proof.
 *
 * \ingroup drat_parsers
 *
 * \param file               Path of the file to be parsed.
 * \param format             If drat_format::text, the file is parsed as text DRAT, otherwise binary DRAT.
 * \param clause_receiver    A function with signature `void(bool, std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. The first argument is true
 *                           if and only if the clause is added to the proof.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename BinaryFn>
void parse_drat_file(std::filesystem::path const& file,
                     drat_format format,
                     BinaryFn&& clause_receiver);

/**
 * Parses a DRAT proof from stdin.
 *
 * \ingroup drat_parsers
 *
 * \param format             If drat_format::text, stdin is parsed as text DRAT, otherwise binary DRAT.
 * \param clause_receiver    A function with signature `void(bool, std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. The first argument is true
 *                           if and only if the clause is added to the proof.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename BinaryFn>
void parse_drat_from_stdin(drat_format format, BinaryFn&& clause_receiver);

/**
 * Parses a text DRAT proof given as a string.
 *
 * \ingroup drat_parsers
 *
 * \param clause_receiver    A function with signature `void(bool, std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. The first argument is true
 *                           if and only if the clause is added to the proof.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename BinaryFn>
void parse_drat_string(std::string const& drat, BinaryFn&& clause_receiver);

/**
 * Parses a binary DRAT proof given as a contiguous sequence of bytes.
 *
 * \ingroup drat_parsers
 *
 * \param clause_receiver    A function with signature `void(bool, std::vector<lit> const&)`.
 *                           `clause_receiver` is invoked for each parsed clause. The first argument is true
 *                           if and only if the clause is added to the proof.
 *
 * \throws std::invalid_argument   when parsing the input failed.
 */
template <typename BinaryFn>
void parse_drat_binary_buffer(std::byte const* start,
                              std::byte const* stop,
                              BinaryFn&& clause_receiver);


// *** Implementation ***

template <typename BinaryFn>
void parse_drat_file(std::filesystem::path const& file,
                     drat_format format,
                     BinaryFn&& clause_receiver)
{
  detail::parse_drat_file_impl(file, format, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_from_stdin(drat_format format, BinaryFn&& clause_receiver)
{
  detail::parse_drat_from_stdin_impl(format, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_string(std::string const& drat, BinaryFn&& clause_receiver)
{
  detail::parse_drat_string_impl(drat, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_binary_buffer(std::byte const* start,
                              std::byte const* stop,
                              BinaryFn&& clause_receiver)
{
  detail::parse_drat_binary_buffer_impl(start, stop, clause_receiver);
}

}
