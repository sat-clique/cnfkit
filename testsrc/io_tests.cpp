#include <cnfkit/io.h>
#include <cnfkit/io/io_buf.h>
#include <cnfkit/io/io_libarchive.h>
#include <cnfkit/io/io_zlib.h>

#include "test_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using ::testing::Eq;

namespace cnfkit {

namespace {
std::string const uncompressed_input = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n";
std::vector<uint8_t> const gz_compressed_input = {
    0x1f, 0x8b, 0x08, 0x08, 0xdd, 0xb5, 0xb8, 0x60, 0x02, 0x03, 0x6c, 0x6f, 0x72, 0x65, 0x6d,
    0x5f, 0x69, 0x70, 0x73, 0x75, 0x6d, 0x00, 0x05, 0xc1, 0xd1, 0x09, 0xc0, 0x20, 0x0c, 0x05,
    0xc0, 0xff, 0x4e, 0xf1, 0x06, 0x28, 0x9d, 0xc4, 0x25, 0x24, 0x06, 0x79, 0x60, 0x8c, 0x24,
    0x71, 0xff, 0xde, 0x35, 0x0f, 0x35, 0xf0, 0xe4, 0x35, 0x0c, 0x5f, 0x1e, 0x48, 0x16, 0xba,
    0x69, 0xbd, 0x10, 0xdf, 0xa9, 0x52, 0x5a, 0x37, 0xd0, 0x07, 0x0f, 0x53, 0xb8, 0x27, 0x74,
    0xb1, 0xbe, 0xe7, 0x07, 0x3a, 0xed, 0x29, 0xfa, 0x39, 0x00, 0x00, 0x00};

void dump_to_file(std::vector<uint8_t> const& content, fs::path const& path)
{
  std::ofstream file{path};
  file.write(reinterpret_cast<char const*>(content.data()), gz_compressed_input.size());
}

auto as_bytes(std::string const& str) -> std::vector<std::byte>
{
  std::vector<std::byte> result;
  for (char const ch : str) {
    result.push_back(static_cast<std::byte>(ch));
  }
  return result;
}

template <typename T>
class decompressing_source_context {
public:
  decompressing_source_context(std::vector<uint8_t> const& input) : m_temp_dir{"cnfkit_io"}
  {
    fs::path const input_file = m_temp_dir.get_path() / "input";
    dump_to_file(input, input_file);
    m_source = std::make_unique<T>(input_file);
  }

  auto get_source() -> T& { return *m_source; }

private:
  std::unique_ptr<T> m_source;
  temp_dir m_temp_dir;
};
}

template <typename T>
class DecompressingSourceTests : public ::testing::Test {
};

using decompressing_sources = ::testing::Types<zlib_source, libarchive_source>;
TYPED_TEST_SUITE(DecompressingSourceTests, decompressing_sources);


namespace {
auto create_buffer() -> std::vector<std::byte>
{
  std::vector<std::byte> result;
  result.resize(uncompressed_input.size());
  return result;
}
}

TYPED_TEST(DecompressingSourceTests, ThrowsOnConstructionWhenFileNotFound)
{
  EXPECT_THROW(TypeParam{"does/not/exist"}, std::runtime_error);
}

TYPED_TEST(DecompressingSourceTests, ReadingCompleteValidInput)
{
  decompressing_source_context<TypeParam> context{gz_compressed_input};
  source& under_test = context.get_source();

  auto buffer = create_buffer();
  auto ptr_past_end =
      under_test.read_bytes(buffer.data(), buffer.data() + uncompressed_input.size());
  buffer.resize(std::distance(buffer.data(), ptr_past_end));

  EXPECT_THAT(buffer, Eq(as_bytes(uncompressed_input)));

  EXPECT_THAT(under_test.read_byte(), Eq(std::nullopt));
  EXPECT_TRUE(under_test.is_eof());
}

TYPED_TEST(DecompressingSourceTests, ReadingCompleteValidInputByteWise)
{
  decompressing_source_context<TypeParam> context{gz_compressed_input};
  source& under_test = context.get_source();

  std::vector<std::byte> result;
  while (!under_test.is_eof()) {
    std::optional<std::byte> const byte = under_test.read_byte();
    if (byte.has_value()) {
      result.push_back(*byte);
    }
  }

  EXPECT_TRUE(under_test.is_eof());
  EXPECT_THAT(result, Eq(as_bytes(uncompressed_input)));
}

TYPED_TEST(DecompressingSourceTests, ReadingAfterMove)
{
  decompressing_source_context<TypeParam> context{gz_compressed_input};
  TypeParam& under_test = context.get_source();

  TypeParam moved_source = std::move(under_test);
  EXPECT_THAT(moved_source.read_byte(), ::testing::Optional(std::byte{'L'}));

  auto buffer = create_buffer();
  EXPECT_THAT(under_test.read_bytes(buffer.data(), buffer.data() + 1), Eq(buffer.data()));
  EXPECT_THAT(under_test.read_byte(), Eq(std::nullopt));
  EXPECT_TRUE(under_test.is_eof());
}

}
