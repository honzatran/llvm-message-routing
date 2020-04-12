

#include <routing/file_util.h>
#include <routing/fmt.h>
#include <boost/algorithm/string/trim.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <string>
#include <string_view>
#include "absl/types/span.h"
#include "routing/buffer.h"

using namespace routing;
using namespace std;

Buffer
routing::read_whole_file_into_buffer(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary | ios::ate);

    if (!ifs)
    {
        return Buffer(0);
    }

    auto size = ifs.tellg();

    Buffer buffer(size);

    ifs.seekg(0, ios::beg);

    ifs.read(buffer.position_ptr_as<char>(), size);

    return buffer;
}

Buffer
routing::read_whole_file_into_buffer(std::string const& filename)
{
    return read_whole_file_into_buffer(filename.c_str());
}

std::string
routing::read_file(char const* filename)
{
    std::ifstream ifs(filename);

    return std::string(
        (std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
}

std::string
routing::replace_extension(std::string_view name, std::string_view ext)
{
    std::string tmp(name.substr(0, name.find_last_of('.') + 1));
    return tmp.append(ext);
}

Open_file::~Open_file()
{
    if (m_file != nullptr)
    {
        fclose(m_file);
        m_file = nullptr;
    }
}

Text_file_writer&
Text_file_writer::append_lines(std::vector<std::string> const& lines)
{
    for (std::string const& line : lines)
    {
        fmt::print(m_file, "{}\n", boost::trim_right_copy(line));
    }

    return *this;
}

Text_file_writer&
Text_file_writer::append_line(std::string const& line)
{
    fmt::print(m_file, "{}\n", boost::trim_right_copy(line));
    return *this;
}

Text_file_writer&
Text_file_writer::new_line()
{
    fmt::print(m_file, "{}\n");
    return *this;
}

void
append_string(routing::Buffer& buffer, std::string& text)
{
    auto buffer_view = routing::make_buffer_view(absl::MakeSpan(text));

    std::uint64_t length     = text.length();
    std::size_t added_length = sizeof(std::uint64_t) + buffer_view.get_length();

    if (buffer.remaining() < added_length)
    {
        buffer.resize(buffer.capacity() + added_length);
    }

    std::size_t dst_index = buffer.get_position();

    fmt::print(
        "Position {} {} {}\n",
        buffer.get_position(),
        buffer.capacity(),
        added_length);

    buffer.set(length, dst_index);
    buffer.copy_from(dst_index + sizeof(std::uint64_t), buffer_view);

    buffer.set_position(dst_index + added_length);
}

std::vector<std::uint8_t>
routing::compress_directory(std::filesystem::path directory_path)
{
    routing::Buffer buffer;

    if (std::filesystem::is_directory(directory_path))
    {
        auto dir_content
            = std::filesystem::recursive_directory_iterator(directory_path);

        for (auto const& entry : dir_content)
        {
            if (entry.is_regular_file())
            {
                fmt::print("Entry {}\n", entry.path());
                std::string path_str
                    = entry.path().lexically_relative(directory_path).string();
                std::string content = read_file(entry.path().c_str());

                append_string(buffer, path_str);
                append_string(buffer, content);

                fmt::print("Position {}\n", buffer.get_position());
            }
        }
    }

    std::size_t size = buffer.get_position();
    std::vector<std::uint8_t> result;

    buffer.swap(result);
    result.resize(size);

    return result;
}

void
routing::decode_directory(
    std::filesystem::path const& output,
    absl::Span<std::uint8_t> data)
{
    std::size_t i = 0;

    Buffer_view view = routing::make_buffer_view(data);

    fmt::print("data {}\n", data.length());

    while (i < data.size())
    {
        auto size = view.as_value<std::uint64_t>(i);
        auto path = std::filesystem::path(
            std::string_view(view.as<char>(i + sizeof(std::uint64_t)), size));

        auto dst_path = output / path;

        if (path.has_parent_path())
        {
            std::filesystem::create_directories(dst_path.parent_path());
        }

        auto content_size
            = view.as_value<std::uint64_t>(i + sizeof(std::uint64_t) + size);

        auto content = std::string_view(
            view.as<char>(i + 2 * sizeof(std::uint64_t) + size), content_size);

        {
            Open_file open_file(dst_path.c_str(), "w+");
            fmt::print(open_file.get_file(), "{}", content);
        }

        i += 2 * sizeof(std::uint64_t) + size + content_size;
    }
}
