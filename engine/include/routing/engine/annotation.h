

#include <absl/strings/string_view.h>

#pragma once

#define ENGINE_FUNCTION __attribute__((annotate("ENGINE")))

#define ENGINE_ROUTER __attribute__((annotate("ENGINE_ROUTER")))

#define ENGINE_FACTORY __attribute__((annotate("ENGINE_FACTORY")))

#define ENGINE_ENTRANCE __attribute__((annotate("ENGINE_ENTRANCE")))

#define ENGINE_OUTPUT __attribute__((annotate("ENGINE_OUTPUT")))

namespace routing::engine
{
constexpr absl::string_view
function_annotation()
{
    return "ENGINE";
}

constexpr absl::string_view
router_annotation()
{
    return "ENGINE_ROUTER";
}

constexpr absl::string_view
router_factory_annotation()
{
    return "ENGINE_FACTORY";
}

constexpr absl::string_view
router_entrance_annotation()
{
    return "ENGINE_ENTRANCE";
}

constexpr absl::string_view
router_output_annotation()
{
    return "ENGINE_OUTPUT";
}

template <typename... ARGS>
void
stub_function(char const* s, ARGS&&... args)
{
}

}  // namespace routing::engine

#ifdef ENGINE_TRANSFORM
#define ENGINE_OUTPUT_CODE(NAME, ...)                      \
    do                                                     \
    {                                                      \
        routing::engine::stub_function(NAME, __VA_ARGS__); \
    } while (false);
#else
#define ENGINE_OUTPUT_CODE(NAME, ...) \
    do                                \
    {                                 \
        g_generated_OUT(__VA_ARGS__); \
    } while (false);
#endif
