#pragma once
#include <cstdio>

#if !defined(FLC_DEBUG_CRASH) && defined(_WIN32)
#include <intrin.h>
#define FLC_DEBUG_CRASH() ::__debugbreak()
#endif

#if !defined(FLC_DEBUG_CRASH) && (defined(__GNUC__) || defined(__clang__))
#define FLC_DEBUG_CRASH() __builtin_trap()
#endif

#if !defined(FLC_DEBUG_CRASH)
#include <cstdlib>
#define FLC_DEBUG_CRASH() ::std::abort()
#endif

#if !defined(NDEBUG)
#define FLC_ASSERT(x, ...)                                    \
    ((void                                                    \
    )(!(x)                                                    \
      && (report_assertion_failure(                           \
              #x, __func__, __FILE__, __LINE__, ##__VA_ARGS__ \
          ),                                                  \
          1)                                                  \
      && (FLC_DEBUG_CRASH(), 1)))
#else
#define FLC_ASSERT(x, ...) ((void)(!(x)))
#endif

namespace
{
template<typename... FormatArgs>
void report_assertion_failure(
    const char* expression,
    const char* function,
    const char* file,
    int line,
    const char* message = "",
    FormatArgs... args
)
{
    std::fprintf(stderr, "Assertion failure: %s\n  ", expression);
    std::fprintf(stderr, message, args...);
    std::fprintf(stderr, "\n  in function %s", function);
    std::fprintf(stderr, "\n  on line %d of file %s\n", line, file);
    std::fflush(stderr);
}
}