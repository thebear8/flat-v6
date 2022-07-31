#include "type.hpp"

bool Type::areSame(Type* a, Type* b)
{
	return a->getResolvedType()->isSame(b->getResolvedType());
}

Type* TypeContext::getNamedType(std::string const& name)
{
	if (!namedTypes.contains(name))
		namedTypes.try_emplace(name, new NamedType(*this, name));
	return namedTypes.at(name);
}

Type* TypeContext::getPointerType(Type* base)
{
	if (!pointerTypes.contains(base))
		pointerTypes.try_emplace(base, new PointerType(base));
	return pointerTypes.at(base);
}

Type* TypeContext::getArrayType(Type* base)
{
	if (!arrayTypes.contains(base))
		arrayTypes.try_emplace(base, new ArrayType(base));
	return arrayTypes.at(base);
}

Type* TypeContext::resolveNamedType(std::string const& name)
{
	if (builtinTypes.contains(name))
		return builtinTypes.at(name);
	else if (structTypes.contains(name))
		return structTypes.at(name);

	throw std::exception("Unknown type");
}

std::string IntegerType::toCppString()
{
	std::string sign = (signedness ? "" : "unsigned ");

	if (bitSize == 8)
		return sign + "char";
	if (bitSize == 16)
		return sign + "short";
	if (bitSize == 32)
		return sign + "int";
	if (bitSize == 64)
		return sign + "long long";

	throw std::exception("Invalid bit size");
}

bool IntegerType::isSame(Type* other)
{
	return dynamic_cast<IntegerType*>(other)
		&& signedness == dynamic_cast<IntegerType*>(other)->signedness
		&& bitSize == dynamic_cast<IntegerType*>(other)->bitSize;
}

bool CharType::isSame(Type* other)
{
	return dynamic_cast<CharType*>(other) &&
		bitSize == dynamic_cast<CharType*>(other)->bitSize;
}

bool StringType::isSame(Type* other)
{
	return dynamic_cast<StringType*>(other);
}

size_t StructType::getBitSize()
{
	size_t bitSize = 0;
	for (auto& [name, type] : fields)
	{
		bitSize += type->getBitSize();
		bitSize = (size_t)(ceil(bitSize / (double)ctx.getPointerSize()) * ctx.getPointerSize()); // align bitSize to pointer size
	}

	return bitSize;
}

bool StructType::isSame(Type* other)
{
	auto o = dynamic_cast<StructType const*>(other);
	if (!o)
		return false;

	if (fields.size() != o->fields.size())
		return false;

	for (size_t i = 0; i < fields.size(); i++)
	{
		if (!Type::areSame(fields[i].second, o->fields[i].second))
			return false;
	}

	return true;
}

bool PointerType::isSame(Type* other)
{
	return dynamic_cast<PointerType const*>(other)
		&& Type::areSame(base, dynamic_cast<PointerType const*>(other)->base);
}

bool ArrayType::isSame(Type* other)
{
	return dynamic_cast<ArrayType const*>(other)
		&& Type::areSame(base, dynamic_cast<ArrayType const*>(other)->base);
}