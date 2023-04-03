#pragma once

#ifndef ENUM_ENTRY
#define ENUM_ENTRY(x) x,
#endif

#ifndef ENUM_NAME_ENTRY
#define ENUM_NAME_ENTRY(x) #x,
#endif

#define DEFINE_ENUM(name, list_name)                     \
    enum class name                                      \
    {                                                    \
        list_name(ENUM_ENTRY)                            \
    };                                                   \
    static constexpr const char* const name##Names[] = { \
        list_name(ENUM_NAME_ENTRY)                       \
    };
