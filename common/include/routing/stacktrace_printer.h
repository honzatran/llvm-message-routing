

#ifndef ROUTING_STACKTRACE_PRINTER_H
#define ROUTING_STACKTRACE_PRINTER_H

#include "stacktrace.h"
#include <routing/stdext.h>

#include <spdlog/fmt/fmt.h>
#include <unistd.h>

namespace routing
{
namespace detail
{
using Frame_array_t
    = Frame_array<Stacktrace_constants::k_stack_trace_max_depth>;
}

class Stdout_printer
{
public:
    void operator()(std::uintptr_t addr, const Symbol_frame& symbol_frame);

    void print_stacktrace();

    template <typename... ARGS>
    void print(char const* format, ARGS... args)
    {
        fmt::print(format, args...);
    }

private:
    std::unique_ptr<detail::Frame_array_t> m_frames;
};

class Signal_safe_printer
{
public:
    Signal_safe_printer(int fd = STDERR_FILENO)
    {
        m_frames = make_unique<detail::Frame_array_t>();
    }

    void operator()(std::uintptr_t addr, const Symbol_frame& symbol_frame);

    void print_stacktrace();

    void flush();

    template <typename... ARGS>
    void print(char const* format, ARGS... args)
    {
        m_memory_writer.write(format, args...);
        write_to_output();
    }

private:
    std::unique_ptr<detail::Frame_array_t> m_frames;

    int m_fd;
    fmt::MemoryWriter m_memory_writer;

    void write_to_output();
};

}  // namespace routing
#endif
