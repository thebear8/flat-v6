#pragma once
#include <ranges>
#include <algorithm>
#include <string>
#include <string_view>
#include <optional>

template<typename TValue>
struct StringSwitch
{
private:
	std::string_view str;
	std::optional<TValue> result;

public:
	StringSwitch(std::string_view str) :
		str(str), result(std::nullopt) { }

	StringSwitch(StringSwitch&& other) :
		str(other.str), result(std::move(other.result)) { }

	StringSwitch(StringSwitch const&) = delete;
	void operator=(StringSwitch&&) = delete;
	void operator=(StringSwitch const&) = delete;
	~StringSwitch() = default;

public:
	StringSwitch& Case(std::string_view s, TValue r)
	{
		result = ((s == str) ? std::move(r) : result);
		return *this;
	}

	StringSwitch& CaseLower(std::string_view s, TValue r)
	{
		std::string left(s), right(str);
		std::transform(left.begin(), left.end(), left.begin(), tolower);
		std::transform(right.begin(), right.end(), right.begin(), tolower);

		result = ((left == right) ? std::move(r) : result);
		return *this;
	}

	StringSwitch& StartsWith(std::string_view s, TValue r)
	{
		result = ((str.starts_with(s)) ? std::move(r) : result);
		return *this;
	}

	StringSwitch& StartsWithLower(std::string_view s, TValue r)
	{
		std::string left(s), right(str);
		std::transform(left.begin(), left.end(), left.begin(), std::tolower);
		std::transform(right.begin(), right.end(), right.begin(), std::tolower);

		result = ((right.starts_with(left)) ? std::move(r) : result);
		return *this;
	}

	StringSwitch& EndsWith(std::string_view s, TValue r)
	{
		result = ((str.ends_with(s)) ? std::move(r) : result);
		return *this;
	}

	StringSwitch& EndsWithLower(std::string_view s, TValue r)
	{
		std::string left(s), right(str);
		std::transform(left.begin(), left.end(), left.begin(), std::tolower);
		std::transform(right.begin(), right.end(), right.begin(), std::tolower);

		result = ((right.ends_with(left)) ? std::move(r) : result);
		return *this;
	}

	TValue Default(TValue value)
	{
		if (result.has_value())
			return std::move(*result);

		return value;
	}

	TValue OrThrow()
	{
		if (!result.has_value())
			throw std::exception("StringSwitch with no matching case");
		return std::move(*result);
	}
};