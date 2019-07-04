

#ifndef ROUTING_STRING_UTILS_H
#define ROUTING_STRING_UTILS_H

#include "buffer.h"
#include "stdext.h"
#include "fmt.h"

#include <boost/utility/string_ref.hpp>

namespace routing
{
inline Buffer_view
convert_to_buffer(boost::string_ref const& str)
{
    char* data = const_cast<char*>(&str[0]);
    return Buffer_view(data, str.size());
}

inline Buffer_view
convert_to_buffer(char const* str)
{
    boost::string_ref tmp = str;
    return convert_to_buffer(tmp);
}

/// Changes the input string in Hungarian notation to the notation of this
/// codebase, for example "MyType" to "My_type"
///
/// \param hungarian input string in hungarian notation
std::string from_hungarian_notation(std::string const& hungarian);

/// Convenient helper class which replaces multiple patterns inside the input 
/// string. This is not efficient.
class String_replacer
{
public:
    String_replacer(std::string const& input)
        : m_input(input)
    {
    }

    /// Replace the pattern with replacement
    String_replacer& replace_with(
        std::string const& pattern,
        std::string const& replacement);

    /// Replace the pattern with replacement
    template <typename T>
    enable_if_t<!std::is_same<decay_t<T>, std::string>::value, String_replacer&>
    replace_with(std::string const& pattern, T const& value)
    {
        return replace_with(pattern, fmt::format("{}", value));
    }

    /// ERASE
    String_replacer& erase(std::string const& pattern);

    std::string const& get_result() const
    {
        return m_input;
    }

private:
    std::string m_input;
};

};  // namespace routing

#endif
