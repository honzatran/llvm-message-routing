

#ifndef ROUTING_STACKTRACE_H
#define ROUTING_STACKTRACE_H

#include <cstdint>
#include <string>

namespace routing
{

struct Stacktrace_constants
{
    constexpr static std::size_t k_stack_trace_max_depth = 100;
};

class Symbol_frame 
{
public:
    Symbol_frame()
    {
        m_symbol.reserve(256);
    }

    void setup(std::uintptr_t offset, char const* symbol);

    bool is_symbol_found() const
    {
        return m_symbol_found;
    }

    std::uintptr_t get_offset() const
    {
        return m_offset;
    }

    std::string const& get_symbol() const
    {
        return m_symbol;
    }

private:
    bool m_symbol_found = false;
    std::uintptr_t m_offset = 0;
    std::string m_symbol;
};

template <std::size_t N>
struct Frame_array
{
public:
    constexpr static std::size_t capacity = N;

    template <typename PRINTER> 
    void print(PRINTER& printer, std::size_t count)
    {
        for (std::size_t i = 0; i < count; i++)
        {
            printer(m_addresses[i], m_frames[i]);
        }
    }

    std::uintptr_t* get_addresses_ptr()
    {
        return &m_addresses[0];
    }

    Symbol_frame* get_frame_ptr()
    {
        return &m_frames[0];
    }

private:
    std::uintptr_t m_addresses[N];
    Symbol_frame m_frames[N];
};

/**
 * Dump the stacktrace into the stdout
 *
 * @return depth of the stacktrace or -1
 */
int stacktrace(
    std::uintptr_t* addresses,
    Symbol_frame* symbol_frame,
    std::size_t addresses_count);

/**
 * Stacktrace which is safe in signal handlers
 *
 * @return depth of the stacktrace or -1
 */
int signal_safe_stacktrace(
    std::uintptr_t* addresses,
    Symbol_frame* symbol_frame,
    std::size_t addresses_count);

};

#endif
