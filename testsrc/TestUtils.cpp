#include "TestUtils.h"

#include <filesystem>
#include <random>
#include <string>

namespace fs = std::filesystem;

namespace cnfkit {
auto operator<<(std::ostream& stream, lit it) -> std::ostream&
{
  stream << (it.is_positive() ? 1 : -1) * static_cast<int64_t>(it.get_var().get_raw_value());
  return stream;
}


temp_dir::temp_dir(std::string const& name_prefix)
{
  fs::path const base_path = fs::temp_directory_path();

  std::mt19937_64 rng{std::random_device{}()};
  std::uniform_int_distribution<size_t> uniform_distribution;

  for (size_t i = 0; i < max_tries; ++i) {
    fs::path candidate = base_path / (name_prefix + std::to_string(uniform_distribution(rng)));
    std::error_code ignored;
    if (fs::create_directory(candidate, ignored)) {
      m_path = candidate;
      break;
    }
  }

  if (m_path.empty()) {
    throw std::runtime_error{"Creating a temporary directory failed"};
  }
}

temp_dir::~temp_dir()
{
  std::error_code ignored;
  fs::remove_all(m_path, ignored);
}

auto temp_dir::get_path() const -> fs::path const&
{
  return m_path;
}

}
