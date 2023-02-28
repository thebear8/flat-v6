#include <concepts>
#include <ranges>
#include <vector>

template<std::ranges::input_range Rng1, std::ranges::input_range Rng2>
    requires std::ranges::view<Rng1> && std::ranges::view<Rng2>
struct zip_view : std::ranges::view_interface<zip_view<Rng1, Rng2>>
{
    using reference = std::pair<
        std::ranges::range_reference_t<Rng1>,
        std::ranges::range_reference_t<Rng2>>;
    using const_reference = std::pair<
        std::ranges::range_reference_t<Rng1>,
        std::ranges::range_reference_t<Rng2>>;
    using value_type = std::pair<
        std::remove_cv_t<
            std::remove_reference_t<std::ranges::range_reference_t<Rng1>>>,
        std::remove_cv_t<
            std::remove_reference_t<std::ranges::range_reference_t<Rng2>>>>;

    struct iterator;
    using const_iterator = iterator;

private:
    std::ranges::iterator_t<Rng1> m_rng1_begin, m_rng1_end;
    std::ranges::iterator_t<Rng2> m_rng2_begin, m_rng2_end;

public:
    zip_view() = default;
    zip_view(Rng1&& rng1, Rng2&& rng2)
        : m_rng1_begin(std::begin(rng1)),
          m_rng1_end(std::end(rng1)),
          m_rng2_begin(std::begin(rng2)),
          m_rng2_end(std::end(rng2))
    {
    }

    zip_view(Rng1 const& rng1, Rng2 const& rng2)
        : m_rng1_begin(std::begin(rng1)),
          m_rng1_end(std::end(rng1)),
          m_rng2_begin(std::begin(rng2)),
          m_rng2_end(std::end(rng2))
    {
    }

    iterator begin() const { return iterator(m_rng1_begin, m_rng2_begin); }
    iterator end() const { return iterator(m_rng1_end, m_rng2_end); }

    const_iterator cbegin() const
    {
        return const_iterator(m_rng1_begin, m_rng2_begin);
    }
    const_iterator cend() const
    {
        return const_iterator(m_rng1_end, m_rng2_end);
    }
};

template<typename Rng1, typename Rng2>
zip_view(
    Rng1&& rng1, Rng2&& rng2
) -> zip_view<std::ranges::views::all_t<Rng1>, std::ranges::views::all_t<Rng2>>;

template<std::ranges::input_range Rng1, std::ranges::input_range Rng2>
    requires std::ranges::view<Rng1> && std::ranges::view<Rng2>
struct zip_view<Rng1, Rng2>::iterator
{
    using difference_type = ptrdiff_t;
    using value_type = zip_view<Rng1, Rng2>::value_type;

private:
    std::ranges::iterator_t<Rng1> m_it1;
    std::ranges::iterator_t<Rng2> m_it2;

public:
    iterator() = default;
    iterator(
        std::ranges::iterator_t<Rng1> it1, std::ranges::iterator_t<Rng2> it2
    )
        : m_it1(std::move(it1)), m_it2(std::move(it2))
    {
    }

    iterator& operator++()
    {
        ++m_it1, ++m_it2;
        return (*this);
    }

    iterator operator++(int) { return ++iterator(m_it1, m_it2); }

    bool operator==(iterator const& other) const
    {
        // check if one of the iterators is equal so we don't iterate past the
        // end in case (m_it1 == end) and (m_it2 != end) or (m_it1 != end) and
        // (m_it2 == end)
        return (m_it1 == other.m_it1) || (m_it2 == other.m_it2);
    }

    bool operator!=(iterator const& other) const
    {
        // check if both of the iterators are unqequal so we don't iterate past
        // the end in case (m_it1 == end) and (m_it2 != end) or (m_it1 != end)
        // and (m_it2 == end)
        return (m_it1 != other.m_it1) && (m_it2 != other.m_it2);
    }

    reference operator*() { return reference(*m_it1, *m_it2); }

    const_reference operator*() const
    {
        return const_reference(*m_it1, *m_it2);
    }

    reference operator->() { return reference(*m_it1, *m_it2); }

    const_reference operator->() const { return reference(*m_it1, *m_it2); }
};

static_assert((bool)std::input_iterator<zip_view<
                  std::views::all_t<std::vector<int>>,
                  std::views::all_t<std::vector<int>>>::iterator>);

static_assert((bool)std::ranges::input_range<zip_view<
                  std::views::all_t<std::vector<int>>,
                  std::views::all_t<std::vector<int>>>>);

static_assert((bool)std::ranges::view<zip_view<
                  std::views::all_t<std::vector<int>>,
                  std::views::all_t<std::vector<int>>>>);