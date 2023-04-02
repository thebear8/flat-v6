#pragma once
#include <cstdint>

struct SourceRef
{
    std::size_t id;
    std::size_t begin, wsBegin;
    std::size_t end, wsEnd;

    SourceRef() : SourceRef(0, 0) {}

    SourceRef(std::size_t id, std::size_t position)
        : SourceRef(id, position, position)
    {
    }

    SourceRef(std::size_t id, std::size_t begin, std::size_t end)
        : SourceRef(id, begin, end, begin, end)
    {
    }

    SourceRef(
        std::size_t id,
        std::size_t begin,
        std::size_t end,
        std::size_t wsBegin,
        std::size_t wsEnd
    )
        : id(id), begin(begin), end(end), wsBegin(wsBegin), wsEnd(wsEnd)
    {
    }
};