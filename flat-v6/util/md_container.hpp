#include <optional>

template<typename... TValues>
struct MetadataContainer;

template<typename TFirst>
struct MetadataContainer<TFirst>
{
    std::optional<TFirst> value;

    template<typename TValue>
    constexpr inline std::optional<TValue> const& getMD();

    template<>
    constexpr inline std::optional<TFirst> const& getMD<TFirst>()
    {
        return value;
    }

    template<typename TValue>
    constexpr inline void setMD(TValue&&);

    template<typename TValue>
    constexpr inline void setMD(TValue const&);

    template<>
    constexpr inline void setMD<TFirst>(TFirst&& v)
    {
        value.emplace(std::forward<TFirst>(v));
    }

    template<>
    constexpr inline void setMD<TFirst>(TFirst const& v)
    {
        value.emplace(std::forward<TFirst>(v));
    }
};

template<typename TFirst, typename... TRest>
struct MetadataContainer<TFirst, TRest...> : MetadataContainer<TRest...>
{
    std::optional<TFirst> value;

    template<typename TValue>
    constexpr inline std::optional<TValue> const& getMD()
    {
        return MetadataContainer<TRest...>::template get<TValue>();
    }

    template<>
    constexpr inline std::optional<TFirst> const& getMD<TFirst>()
    {
        return value;
    }

    template<typename TValue>
    constexpr inline void set(TValue&& v)
    {
        return MetadataContainer<TRest...>::template setMD<TValue>(
            std::forward<TValue>(v)
        );
    }

    template<typename TValue>
    constexpr inline void set(TValue const& v)
    {
        return MetadataContainer<TRest...>::template setMD<TValue>(
            std::forward<TValue>(v)
        );
    }

    template<>
    constexpr inline void setMD(TFirst&& v)
    {
        value.emplace(std::forward<TFirst>(v));
    }

    template<>
    constexpr inline void setMD(TFirst const& v)
    {
        value.emplace(std::forward<TFirst>(v));
    }
};