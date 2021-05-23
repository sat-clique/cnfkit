#pragma once

#include <cnfkit/detail/CnfLikeParser.h>

#include <filesystem>
#include <string>
#include <vector>

namespace cnfkit::detail {

template <typename It>
auto parse_drat_binary_lit(It start, It stop) -> std::pair<lit, It>
{
  if (start == stop) {
    throw std::invalid_argument{"invalid binary drat literal"};
  }

  uint32_t raw_lit = 0;
  uint32_t shift = 0;
  bool found_end = false;
  It cursor = start;

  do {
    if (shift == 28 && (*cursor & 0x7F) > 0x0F) {
      throw std::invalid_argument{"literal out of range"};
    }

    raw_lit |= ((*cursor & 0x7F) << shift);
    shift += 7;
    found_end = (*cursor & 0x80) == 0;
    ++cursor;
  } while (!found_end && cursor != stop && shift <= 28);

  if (!found_end) {
    throw std::invalid_argument{"unexpected end of binary drat literal"};
  }

  uint32_t const raw_cnfkit_var = raw_lit / 2;

  if (raw_cnfkit_var > max_raw_var) {
    throw std::invalid_argument{"literal out of range"};
  }

  lit result{var{raw_cnfkit_var}, (raw_lit & 1) == 0};

  return {result, cursor};
}

class drat_binary_chunk_parser {
public:
  template <typename It, typename BinaryFn>
  void parse(It start, It stop, BinaryFn&& clause_receiver)
  {
    It cursor = start;
    while (cursor != stop) {
      if (*cursor == 0x61) {
        m_is_in_add_mode = true;
        m_is_in_clause = true;
        ++cursor;
      }
      else if (*cursor == 0x64) {
        m_is_in_add_mode = false;
        m_is_in_clause = true;
        ++cursor;
      }
      else if (*cursor == 0) {
        if (!m_is_in_clause) {
          throw std::invalid_argument{"clause not preceded by a or d"};
        }

        clause_receiver(m_is_in_add_mode, m_lit_buffer);
        m_is_in_clause = false;
        m_lit_buffer.clear();
        ++cursor;
      }
      else {
        if (!m_is_in_clause) {
          throw std::invalid_argument{"clause not preceded by a or d"};
        }

        auto const [lit, next] = parse_drat_binary_lit(cursor, stop);
        cursor = next;
        m_lit_buffer.push_back(lit);
      }
    }
  }

  void check_on_drat_finish()
  {
    if (m_is_in_clause) {
      throw std::invalid_argument{"unexpected end of proof"};
    }
  }

private:
  std::vector<lit> m_lit_buffer;
  bool m_is_in_add_mode = false;
  bool m_is_in_clause = false;
};

template <typename It>
inline bool is_binary_drat(It start, It stop)
{
  // TODO: only scan the first 10 chars
  return std::find(start, stop, 0) != stop;
}

template <typename BinaryFn>
auto parse_drat_text_gz_file(cnf_gz_file& file, BinaryFn&& clause_receiver)
{
  cnf_chunk_parser parser{cnf_chunk_parser_mode::drat};

  std::string buffer;
  while (!file.is_eof()) {
    file.read_chunk(default_chunk_size, buffer);
    parser.parse(buffer, 0, clause_receiver);
  }

  parser.check_on_drat_finish();
}

template <typename BinaryFn>
void parse_drat_file_impl(std::filesystem::path const& input_file, BinaryFn&& clause_receiver)
{
  cnf_gz_file file{input_file};
  parse_drat_text_gz_file(file, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_from_stdin_impl(BinaryFn&& clause_receiver)
{
  cnf_gz_file stdin_file;
  parse_drat_text_gz_file(stdin_file, clause_receiver);
}

template <typename BinaryFn>
void parse_drat_string_impl(std::string const& drat, BinaryFn&& clause_receiver)
{
  if (is_binary_drat(drat.begin(), drat.end())) {
    drat_binary_chunk_parser parser;
    parser.parse(drat.begin(), drat.end(), clause_receiver);
    parser.check_on_drat_finish();
  }
  else {
    cnf_chunk_parser parser{cnf_chunk_parser_mode::drat};
    parser.parse(drat, 0, clause_receiver);
    parser.check_on_drat_finish();
  }
}
}
