

#include <routing/file_util.h>
#include <routing/fmt.h>
#include <boost/algorithm/string/trim.hpp>

#include <fstream>
#include <string>


using namespace routing;
using namespace std;

Buffer 
routing::read_whole_file_into_buffer(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary| ios::ate);

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
        (std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
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
