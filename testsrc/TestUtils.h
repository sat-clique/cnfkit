#pragma once

#include <cnfkit/Literal.h>

#include <filesystem>
#include <iosfwd>
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
