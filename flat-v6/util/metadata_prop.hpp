#include <optional>

#define METADATA_PROP(name, type, get, set)                       \
private:                                                          \
    std::optional<type> metadata_##name;                          \
                                                                  \
public:                                                           \
    type& get()                                                   \
    {                                                             \
        return (metadata_##name.value());                         \
    }                                                             \
                                                                  \
    type const& get() const                                       \
    {                                                             \
        return (metadata_##name.value());                         \
    }                                                             \
                                                                  \
    type const& get(type&& default_) const                        \
    {                                                             \
        return (metadata_##name.has_value())                      \
            ? (std::forward<type const>(metadata_##name.value())) \
            : (std::forward<type const>(default_));               \
    }                                                             \
                                                                  \
    type const& get(type const& default_) const                   \
    {                                                             \
        return (metadata_##name.has_value())                      \
            ? (std::forward<type const>(metadata_##name.value())) \
            : (std::forward<type const>(default_));               \
    }                                                             \
                                                                  \
    auto set(type&& value)                                        \
    {                                                             \
        metadata_##name.emplace(std::forward<type>(value));       \
        return this;                                              \
    }                                                             \
                                                                  \
    auto set(type const& value)                                   \
    {                                                             \
        metadata_##name.emplace(std::forward<type const>(value)); \
        return this;                                              \
    }
