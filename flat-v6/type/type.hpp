#pragma once
#include <string>
#include <unordered_map>

class Type;
class TypeContext;
class VoidType;
class BoolType;
class IntegerType;
class CharType;
class StringType;
class StructType;
class PointerType;
class ArrayType;

class Type
{
public:
	TypeContext& ctx;

public:
	Type(TypeContext& ctx) :
		ctx(ctx)
	{
	}

public:
	virtual size_t getBitSize() = 0;
	virtual std::string toString() = 0;

	virtual bool isVoidType() { return false; }
	virtual bool isBoolType() { return false; }
	virtual bool isIntegerType() { return false; }
	virtual bool isCharType() { return false; }
	virtual bool isStringType() { return false; }
	virtual bool isStructType() { return false; }
	virtual bool isPointerType() { return false; }
	virtual bool isArrayType() { return false; }
	virtual bool isSigned() { return false; }
};

class TypeContext
{
private:
	size_t pointerSize;
	std::unordered_map<Type*, PointerType*> pointerTypes;
	std::unordered_map<Type*, ArrayType*> arrayTypes;
	std::unordered_map<std::string, StructType*> structTypes;
	std::unordered_map<std::string, Type*> builtinTypes;

public:
	TypeContext();
	~TypeContext();

public:
	size_t getPointerSize() { return pointerSize; }
	size_t setPointerSize(size_t size) { return (pointerSize = size); }

	PointerType* getPointerType(Type* base);
	ArrayType* getArrayType(Type* base);
	StructType* getStructType(std::string const& name);
	Type* getResolvedType(std::string const& name);
};

class VoidType : public Type
{
public:
	VoidType(TypeContext& ctx) :
		Type(ctx) { }

public:
	virtual size_t getBitSize() override { return 0; };
	virtual std::string toString() override { return "void"; };
	virtual bool isVoidType() override { return true; }
};

class BoolType : public Type
{
public:
	BoolType(TypeContext& ctx) :
		Type(ctx) { }

public:
	virtual size_t getBitSize() override { return 1; };
	virtual std::string toString() override { return "bool"; };
	virtual bool isBoolType() override { return true; }
};

class IntegerType : public Type
{
public:
	bool signedness;
	size_t bitSize;

public:
	IntegerType(TypeContext& ctx, bool signedness, size_t bitSize) :
		Type(ctx), signedness(signedness), bitSize(bitSize) { }

public:
	virtual size_t getBitSize() override { return bitSize; };
	virtual std::string toString() override { return (signedness ? "i" : "u") + std::to_string(bitSize); };
	virtual bool isIntegerType() override { return true; }
	virtual bool isSigned() override { return signedness; }
};

class CharType : public Type
{
public:
	size_t bitSize;

public:
	CharType(TypeContext& ctx, size_t bitSize) :
		Type(ctx), bitSize() { }

public:
	virtual size_t getBitSize() override { return bitSize; };
	virtual std::string toString() override { return "char"; };
	virtual bool isCharType() override { return true; }
};

class StringType : public Type
{
public:
	StringType(TypeContext& ctx) :
		Type(ctx) { }

public:
	virtual size_t getBitSize() override { return ctx.getPointerSize(); };
	virtual std::string toString() override { return "str"; };
	virtual bool isStringType() override { return true; }
};

class StructType : public Type
{
public:
	std::string name;
	std::vector<std::pair<std::string, Type*>> fields;

public:
	StructType(TypeContext& ctx, std::string const& name, std::vector<std::pair<std::string, Type*>> const& members = {}) :
		Type(ctx), name(name), fields(members) { }

public:
	virtual size_t getBitSize() override;
	virtual std::string toString() override { return name; };
	virtual bool isStructType() override { return true; }

	void addField(std::string const& name, Type* type) { return fields.push_back(std::pair(name, type)); }
};

class PointerType : public Type
{
public:
	Type* base;

public:
	PointerType(Type* base) :
		Type(base->ctx), base(base) { }

public:
	virtual size_t getBitSize() override { return ctx.getPointerSize(); };
	virtual std::string toString() override { return base->toString() + "*"; };
	virtual bool isPointerType() override { return true; }
};

class ArrayType : public Type
{
public:
	Type* base;

public:
	ArrayType(Type* base) :
		Type(base->ctx), base(base) { }

public:
	virtual size_t getBitSize() override { return ctx.getPointerSize(); };
	virtual std::string toString() override { return base->toString() + "[]"; };
	virtual bool isArrayType() override { return true; }
};