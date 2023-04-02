#include <ranges>
#include <vector>

namespace range_utils
{
struct to_vector_closure
{
    template<std::ranges::range Rng>
    constexpr auto operator()(Rng&& rng) const
    {
        auto rngCommon = rng | std::views::common;
        std::vector<std::ranges::range_value_t<Rng>> vec;

        if constexpr (requires { std::ranges::size(rng); })
            vec.reserve(std::ranges::size(rng));

        vec.insert(vec.begin(), rngCommon.begin(), rngCommon.end());
        return vec;
    }
};

inline to_vector_closure to_vector;

template<std::ranges::range Rng>
constexpr auto operator|(Rng&& rng, to_vector_closure const& closure)
{
    return closure(std::forward<Rng>(rng));
}
}