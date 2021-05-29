#pragma once

#include <cnfkit/detail/CheckCxxVersion.h>

#include <cnfkit/Literal.h>
#include <cnfkit/detail/DratParser.h>

#include <cstddef>
#include <filesystem>
#include <string>

namespace cnfkit {

template <typename BinaryFn>
void parse_drat_file(std::filesystem::path const& file,
                     drat_format format,
                     BinaryFn&& clause_receiver);

template <typename BinaryFn>
void parse_drat_from_stdin(drat_format format, BinaryFn&& clause_receiver);

template <typename BinaryFn>
void parse_drat_string(std::string const& drat, BinaryFn&& clause_receiver);

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
