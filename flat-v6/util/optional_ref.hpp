#pragma once
#include <functional>
#include <optional>

template<typename T>
class optional_ref
{
private:
    std::optional<std::reference_wrapper<T>> m_ref;

public:
    optional_ref() : m_ref(std::nullopt) {}
    optional_ref(std::nullopt_t) : m_ref(std::nullopt) {}
    optional_ref(T& value) : m_ref(value) {}

    optional_ref& operator=(T&& value)
    {
        get() = value;
        return *this;
    }

    T& get() { return m_ref.value().get(); }
    T const& get() const { return m_ref.value().get(); }
    bool has_value() const noexcept { return m_ref.has_value(); }
    T& value_or(T& other) { return (has_value() ? get() : other); }
    T const& value_or(T const& other) const
    {
        return (has_value() ? get() : other);
    }

    friend bool operator==(optional_ref const& a, optional_ref const& b)
    {
        if (a.has_value() != b.has_value())
        {
            return false;
        }
        else if (!a.has_value())
        {
            return true;
        }
        else
        {
            return a.get() == b.get();
        }
    }

    T& operator*() { return get(); }
    T const& operator*() const { return get(); }

    T* operator->() { return &get(); }
    T const* operator->() const { return &get(); }

    operator T&() { return get(); }
    operator T const&() const { return get(); }
    operator bool() const noexcept { return has_value(); }
};