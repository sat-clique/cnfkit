#pragma once

#include <cnfkit/literal.h>

#include <filesystem>
#include <ostream>
#include <string>


namespace cnfkit {
auto operator<<(std::ostream& stream, lit it) -> std::ostream&;

class temp_dir {
public:
  constexpr static size_t max_tries = 1024;

  explicit temp_dir(std::string const& name_prefix);
  ~temp_dir();
  auto get_path() const -> std::filesystem::path const&;

private:
  std::filesystem::path m_path;
};
}
