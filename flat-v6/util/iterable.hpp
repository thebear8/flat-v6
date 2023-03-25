#pragma once
#include <ranges>
#include <vector>

template<typename TRange, typename TElement>
concept Iterable = std::ranges::range<TRange>
    && std::same_as<TElement, std::ranges::range_value_t<TRange>>;

static_assert(Iterable<std::vector<int>, int>);
static_assert(!Iterable<std::vector<std::nullptr_t>, int>);