#include "type.hpp"

TypeContext::TypeContext() :
	pointerSize()
{
	builtinTypes.try_emplace("void", new VoidType(*this));
	builtinTypes.try_emplace("bool", new BoolType(*this));
	builtinTypes.try_emplace("u8", new IntegerType(*this, false, 8));
	builtinTypes.try_emplace("u16", new IntegerType(*this, false, 16));
	builtinTypes.try_emplace("u32", new IntegerType(*this, false, 32));
	builtinTypes.try_emplace("u64", new IntegerType(*this, false, 64));
	builtinTypes.try_emplace("i8", new IntegerType(*this, true, 8));
	builtinTypes.try_emplace("i16", new IntegerType(*this, true, 16));
	builtinTypes.try_emplace("i32", new IntegerType(*this, true, 32));
	builtinTypes.try_emplace("i64", new IntegerType(*this, true, 64));
	builtinTypes.try_emplace("char", new CharType(*this, 32));
	builtinTypes.try_emplace("str", new StringType(*this));
}

TypeContext::~TypeContext()
{
	for (auto& [base, type] : pointerTypes)
		delete type;
	for (auto& [base, type] : arrayTypes)
		delete type;
	for (auto& [name, type] : structTypes)
		delete type;
	for (auto& [name, type] : builtinTypes)
		delete type;
}

PointerType* TypeContext::getPointerType(Type* base)
{
	if (!pointerTypes.contains(base))
		pointerTypes.try_emplace(base, new PointerType(base));
	return pointerTypes.at(base);
}

ArrayType* TypeContext::getArrayType(Type* base)
{
	if (!arrayTypes.contains(base))
		arrayTypes.try_emplace(base, new ArrayType(base));
	return arrayTypes.at(base);
}

StructType* TypeContext::getStructType(std::string const& name)
{
	if (!structTypes.contains(name))
		structTypes.try_emplace(name, new StructType(*this, name, {}));
	return structTypes.at(name);
}

Type* TypeContext::getResolvedType(std::string const& name)
{
	if (builtinTypes.contains(name))
		return builtinTypes.at(name);
	else if (structTypes.contains(name))
		return structTypes.at(name);
	return nullptr;
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