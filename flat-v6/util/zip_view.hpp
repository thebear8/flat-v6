#include <concepts>
#include <ranges>
#include <vector>

template<std::ranges::input_range Rng1, std::ranges::input_range Rng2>
struct zip_view : std::ranges::view_interface<zip_view<Rng1, Rng2>>
{
    template<typename TRange>
    using range_iterator_t = decltype(std::begin(std::declval<TRange&>()));
    template<typename TRange>
    using range_const_iterator_t =
        decltype(std::cbegin(std::declval<TRange&>()));
    template<typename TRange>
    using range_reference_t = decltype(*std::begin(std::declval<TRange&>()));
    template<typename TRange>
    using range_const_reference_t =
        decltype(*std::cbegin(std::declval<TRange&>()));

    using reference =
        std::pair<range_reference_t<Rng1>, range_reference_t<Rng2>>;
    using const_reference =
        std::pair<range_const_reference_t<Rng1>, range_const_reference_t<Rng2>>;
    using value_type = std::pair<
        std::remove_cvref_t<range_reference_t<Rng1>>,
        std::remove_cvref_t<range_reference_t<Rng2>>>;

    struct iterator;
    struct const_iterator;

private:
    Rng1 m_rng1;
    Rng2 m_rng2;

public:
    zip_view() = default;
    zip_view(Rng1&& rng1, Rng2&& rng2)
        : m_rng1(std::move(rng1)), m_rng2(std::move(rng2))
    {
    }

    zip_view(Rng1 const& rng1, Rng2 const& rng2) : m_rng1(rng1), m_rng2(rng2) {}

    iterator begin()
    {
        return iterator(std::begin(m_rng1), std::begin(m_rng2));
    }
    iterator end() { return iterator(std::end(m_rng1), std::end(m_rng2)); }

    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

    const_iterator cbegin() const
    {
        return const_iterator(std::cbegin(m_rng1), std::cbegin(m_rng2));
    }
    const_iterator cend() const
    {
        return const_iterator(std::cend(m_rng1), std::cend(m_rng2));
    }
};

template<typename Rng1, typename Rng2>
zip_view(Rng1 const&, Rng2 const&) -> zip_view<Rng1, Rng2>;

template<std::ranges::input_range Rng1, std::ranges::input_range Rng2>
struct zip_view<Rng1, Rng2>::iterator
{
    using difference_type = ptrdiff_t;
    using value_type = zip_view<Rng1, Rng2>::value_type;

private:
    range_iterator_t<Rng1> m_it1;
    range_iterator_t<Rng2> m_it2;

public:
    iterator() = default;
    iterator(range_iterator_t<Rng1> it1, range_iterator_t<Rng2> it2)
        : m_it1(std::move(it1)), m_it2(std::move(it2))
    {
    }

    iterator& operator++()
    {
        ++m_it1, ++m_it2;
        return (*this);
    }

    iterator operator++(int) const { return ++iterator(m_it1, m_it2); }

    bool operator==(iterator const& other) const
    {
        // check if one of the iterators is equal so we don't iterate past the
        // end in case (m_it1 == end) and (m_it2 != end) or (m_it1 != end) and
        // (m_it2 == end)
        return (m_it1 == other.m_it1) || (m_it2 == other.m_it2);
    }

    reference operator*() const { return reference(*m_it1, *m_it2); }
    reference operator->() const { return reference(*m_it1, *m_it2); }
};

template<std::ranges::input_range Rng1, std::ranges::input_range Rng2>
struct zip_view<Rng1, Rng2>::const_iterator
{
    using difference_type = ptrdiff_t;
    using value_type = zip_view<Rng1, Rng2>::value_type;

private:
    range_const_iterator_t<Rng1> m_it1;
    range_const_iterator_t<Rng2> m_it2;

public:
    const_iterator() = default;
    const_iterator(
        range_const_iterator_t<Rng1> it1, range_const_iterator_t<Rng2> it2
    )
        : m_it1(std::move(it1)), m_it2(std::move(it2))
    {
    }

    const_iterator& operator++()
    {
        ++m_it1, ++m_it2;
        return (*this);
    }

    const_iterator operator++(int) const
    {
        return ++const_iterator(m_it1, m_it2);
    }

    bool operator==(const_iterator const& other) const
    {
        // check if one of the iterators is equal so we don't iterate past the
        // end in case (m_it1 == end) and (m_it2 != end) or (m_it1 != end) and
        // (m_it2 == end)
        return (m_it1 == other.m_it1) || (m_it2 == other.m_it2);
    }

    const_reference operator*() const
    {
        return const_reference(*m_it1, *m_it2);
    }

    const_reference operator->() const
    {
        return const_reference(*m_it1, *m_it2);
    }
};

static_assert(std::input_iterator<
              zip_view<std::vector<int>, std::vector<int>>::iterator>);

static_assert(std::ranges::input_range<
              zip_view<std::vector<int>, std::vector<int>>>);

static_assert(std::ranges::view<zip_view<std::vector<int>, std::vector<int>>>);
