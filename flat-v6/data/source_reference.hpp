#pragma once

struct SourceReference
{
	size_t sourceId;
	size_t whitespaceBegin;
	size_t begin;
	size_t end;
	size_t whitespaceEnd;
};