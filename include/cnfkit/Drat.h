#pragma once

#include <cnfkit/Literal.h>
#include <cnfkit/detail/DratParser.h>

#include <filesystem>
#include <string>

namespace cnfkit {

template <typename BinaryFn>
void parse_drat_file(std::filesystem::path const& file, BinaryFn&& clause_receiver);

template <typename BinaryFn>
void parse_drat_from_stdin(BinaryFn&& clause_receiver);

template <typename BinaryFn>
void parse_drat_string(std::string const& drat, BinaryFn&& clause_receiver);


// *** Implementation ***

template <typename BinaryFn>
void parse_drat_file(std::filesystem::path const& file, BinaryFn&& clause_receiver)
{
  detail::parse_drat_file_impl(file, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_from_stdin(BinaryFn&& clause_receiver)
{
  detail::parse_drat_from_stdin_impl(clause_receiver);
}

template <typename BinaryFn>
void parse_drat_string(std::string const& drat, BinaryFn&& clause_receiver)
{
  detail::parse_drat_string_impl(drat, clause_receiver);
}
}
