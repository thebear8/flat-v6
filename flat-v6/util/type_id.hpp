#pragma once

struct TypeId
{
private:
	const void** value;

public:
	constexpr inline TypeId() : value(nullptr) { }
	constexpr inline TypeId(const void** value) : value(value) { }

public:
	constexpr inline bool operator==(TypeId const& other) const noexcept { return value == other.value; }

private:
	template<typename Type>
	struct Id { static const inline void* id = nullptr; };

public:
	template<typename Type>
	static consteval inline TypeId get() { return std::forward<TypeId>(TypeId(&Id<Type>::id)); }
};

template<typename Type>
static consteval TypeId type_id()
{
	return std::forward<TypeId>(TypeId::get<Type>());
}