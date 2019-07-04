

#include <routing/stacktrace_printer.h>

#include <spdlog/fmt/fmt.h>
#include <cxxabi.h>
#include <unistd.h>
#include <stdio.h>

using namespace routing;

void
Stdout_printer::operator()(std::uintptr_t addr, const Symbol_frame& symbol_frame)
{
    fmt::print("{:#X}: ", addr);

    if (symbol_frame.is_symbol_found())
    {
        fmt::print(
            "({}+{:#X})\n",
            symbol_frame.get_symbol(),
            symbol_frame.get_offset());
    }
    else
    {
        fmt::print(" -- error: unable to obtain symbol for this frame\n");
    }
}

void 
Stdout_printer::print_stacktrace()
{
    int stacktrace_depth = stacktrace(
        m_frames->get_addresses_ptr(),
        m_frames->get_frame_ptr(),
        m_frames->capacity);

    if (stacktrace_depth < 0)
    {
        return;
    }

    m_frames->print(*this, stacktrace_depth);
}

void
Signal_safe_printer::operator()(std::uintptr_t addr, const Symbol_frame& symbol_frame)
{
    m_memory_writer.write("{:#X}: ", addr);

    if (symbol_frame.is_symbol_found())
    {
        m_memory_writer.write(
            "({}+{:#X})\n",
            symbol_frame.get_symbol(),
            symbol_frame.get_offset());
    }
    else
    {
        m_memory_writer.write(
            " -- error: unable to obtain symbol for this frame\n");
    }

    write_to_output();
}

void 
Signal_safe_printer::print_stacktrace()
{
    int stacktrace_depth = signal_safe_stacktrace(
        m_frames->get_addresses_ptr(),
        m_frames->get_frame_ptr(),
        m_frames->capacity);

    if (stacktrace_depth < 0)
    {
        return;
    }

    m_frames->print(*this, stacktrace_depth);
}

void 
Signal_safe_printer::flush()
{
    fsync(m_fd);
}

void Signal_safe_printer::write_to_output()
{
    void const* data = reinterpret_cast<void const*>(m_memory_writer.data());
    std::size_t size = m_memory_writer.size() * sizeof(char);

    write(m_fd, data, size);

    m_memory_writer.clear();
}

