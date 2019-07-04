

#include <routing/string_utils.h>

#include <cctype>

#include <boost/algorithm/string/replace.hpp>

using namespace routing;
using namespace std;

bool is_upper_character(char c)
{
    return std::isalpha(c) && std::islower(c) == 0;
}

std::string
routing::from_hungarian_notation(std::string const& hungarian)
{
    if (hungarian.empty())
    {
        return "";
    }

    std::string transformed_name;

    transformed_name.push_back(hungarian[0]);

    bool is_previous_letter_upper = is_upper_character(hungarian[0]);

    for (char c : hungarian.substr(1))
    {
        if (std::isalpha(c) && !std::islower(c))
        {
            if (is_previous_letter_upper)
            {
                transformed_name.push_back(std::tolower(c));
            }
            else 
            {
                transformed_name.push_back('_');
                transformed_name.push_back(std::tolower(c));
            }
        }
        else
        {
            transformed_name.push_back(c);
        }

        is_previous_letter_upper = is_upper_character(c);
    }

    return transformed_name;
}

String_replacer&
String_replacer::replace_with(
    std::string const& pattern,
    std::string const& replacement)
{
    boost::replace_all(m_input, pattern, replacement);

    return *this;
}

String_replacer& String_replacer::erase(std::string const& pattern)
{
    return replace_with(pattern, "");
}
