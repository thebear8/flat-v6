#pragma once

struct SourceRef
{
    size_t id;
    size_t begin, wsBegin;
    size_t end, wsEnd;

    SourceRef() : SourceRef(0, 0) {}

    SourceRef(size_t id, size_t position) : SourceRef(id, position, position) {}

    SourceRef(size_t id, size_t begin, size_t end)
        : SourceRef(id, begin, end, begin, end)
    {
    }

    SourceRef(size_t id, size_t begin, size_t end, size_t wsBegin, size_t wsEnd)
        : id(id), begin(begin), end(end), wsBegin(wsBegin), wsEnd(wsEnd)
    {
    }
};