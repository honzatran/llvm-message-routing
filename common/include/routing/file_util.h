

#ifndef ROUTING_FILE_UTIL_H
#define ROUTING_FILE_UTIL_H

#include <routing/buffer.h>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

namespace routing
{
Buffer
read_whole_file_into_buffer(char const* filename);
Buffer
read_whole_file_into_buffer(std::string const& filename);

/// Reads the whole content of the file into the string
std::string
read_file(char const* filename);

/// RAII wrapper around FILE* for IO
class Open_file
{
public:
    Open_file(std::string const& path, std::string const& flags)
    {
        m_file = fopen(path.c_str(), flags.c_str());
    }

    Open_file(Open_file const& other) = delete;
    Open_file& operator=(Open_file const& other) = delete;

    Open_file(Open_file&& other) = default;
    Open_file& operator=(Open_file&& other) = default;

    ~Open_file();

    FILE* get_file() { return m_file; }

    operator FILE*() { return get_file(); }

private:
    FILE* m_file;
};

/// Write tag for Text formatter to a file
/// File will be created if it does not exists. The writes always start
/// at the start.
struct Write_tag
{
};

/// Append tag for Text formatter to a file
/// File will be created if it does not exists. The writes are appended
/// at the end of the file.
struct Append_tag
{
};

/// Text formatter to a file
class Text_file_writer
{
public:
    Text_file_writer(std::string const& path, Write_tag) : m_file(path, "w+") {}

    Text_file_writer(std::string const& path, Append_tag) : m_file(path, "a+")
    {
    }

    Text_file_writer& append_lines(std::vector<std::string> const& lines);
    Text_file_writer& append_line(std::string const& lines);
    Text_file_writer& new_line();

private:
    Open_file m_file;
};

std::string
replace_extension(std::string_view name, std::string_view ext);

std::vector<std::uint8_t>
compress_directory(std::filesystem::path directory_path);

void
decode_directory(
    std::filesystem::path const& output,
    absl::Span<std::uint8_t> data);

}  // namespace routing

#endif
