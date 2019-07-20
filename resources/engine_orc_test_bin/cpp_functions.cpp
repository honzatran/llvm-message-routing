

#include <numeric>
#include <vector>
#include <algorithm>

std::vector<int>
generate_vector(std::size_t size)
{
    std::vector<int> output(size);

    for (std::size_t i = 0; i < size; ++i)
    {
        output[i] = 2 * i * i + 42;
    }

    return output;
}

extern "C" int
test_cpp_function(int init_value, std::size_t value)
{
    auto output = generate_vector(value);

    return std::accumulate(output.begin(), output.end(), init_value);
}
