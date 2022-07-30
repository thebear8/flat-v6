#pragma once
#include <string>
#include <unordered_map>

class TypeContext;

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
	virtual Type* getResolvedType() = 0;
	virtual std::string toString() = 0;
	virtual std::string toCppString() = 0;
	virtual bool isSame(Type* other) = 0;

	virtual bool isVoidType() { return false; }
	virtual bool isBoolType() { return false; }
	virtual bool isIntegerType() { return false; }
	virtual bool isCharType() { return false; }
	virtual bool isStringType() { return false; }
	virtual bool isStructType() { return false; }
	virtual bool isPointerType() { return false; }
	virtual bool isArrayType() { return false; }

	virtual bool isSigned() { return false; }

public:
	static bool areSame(Type* a, Type* b);
};

class TypeContext
{
public:
	size_t pointerSize;

	std::unordered_map<std::string, Type*> namedTypes;
	std::unordered_map<std::string, Type*> builtinTypes;
	std::unordered_map<std::string, Type*> structTypes;
	std::unordered_map<Type*, Type*> pointerTypes;
	std::unordered_map<Type*, Type*> arrayTypes;

public:
	TypeContext(size_t pointerSize) :
		pointerSize(pointerSize) { }

public:
	Type* getNamedType(std::string const& name);
	Type* getPointerType(Type* base);
	Type* getArrayType(Type* base);
	Type* resolveNamedType(std::string const& name);

	bool hasNamedType(std::string const& name);
};

class VoidType : public Type
{
public:
	VoidType(TypeContext& ctx) :
		Type(ctx) { }

public:
	virtual size_t getBitSize() override { return 0; };
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return "void"; };
	virtual std::string toCppString() override { return "void"; };
	virtual bool isSame(Type* other) override { return dynamic_cast<VoidType*>(other); };
	virtual bool isVoidType() override { return true; }
};

class BoolType : public Type
{
public:
	BoolType(TypeContext& ctx) :
		Type(ctx) { }

public:
	virtual size_t getBitSize() override { return 1; };
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return "bool"; };
	virtual std::string toCppString() override { return "bool"; };
	virtual bool isSame(Type* other) override { return dynamic_cast<BoolType*>(other); };
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
	virtual Type* getResolvedType() override { return this; }
	virtual std::string toString() override { return (signedness ? "i" : "u") + std::to_string(bitSize); };
	virtual std::string toCppString() override;
	virtual bool isSame(Type* other) override;
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
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return "char"; };
	virtual std::string toCppString() override { return "char"; };
	virtual bool isSame(Type* other) override;
	virtual bool isCharType() override { return true; }
};

class StringType : public Type
{
public:
	StringType(TypeContext& ctx) :
		Type(ctx) { }

public:
	virtual size_t getBitSize() override { return ctx.pointerSize; };
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return "str"; };
	virtual std::string toCppString() override { return "const char*"; };
	virtual bool isSame(Type* other) override;
	virtual bool isStringType() override { return true; }
};

class StructType : public Type
{
public:
	std::string name;
	std::vector<std::pair<std::string, Type*>> fields;

public:
	StructType(TypeContext& ctx, std::string name, std::vector<std::pair<std::string, Type*>> members) :
		Type(ctx), name(name), fields(members) { }

public:
	virtual size_t getBitSize() override;
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return name; };
	virtual std::string toCppString() override { return "struct " + name; };
	virtual bool isSame(Type* other) override;
	virtual bool isStructType() override { return true; }
};

class PointerType : public Type
{
public:
	Type* base;

public:
	PointerType(Type* base) :
		Type(base->ctx), base(base) { }

public:
	virtual size_t getBitSize() override { return ctx.pointerSize; };
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return base->toString() + "*"; };
	virtual std::string toCppString() override { return base->toCppString() + "*"; };
	virtual bool isSame(Type* other) override;
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
	virtual size_t getBitSize() override { return ctx.pointerSize; };
	virtual Type* getResolvedType() override { return this; };
	virtual std::string toString() override { return base->toString() + "[]"; };
	virtual std::string toCppString() override { return base->toCppString() + "*"; };
	virtual bool isSame(Type* other) override;
	virtual bool isArrayType() override { return true; }
};

class NamedType : public Type
{
public:
	std::string name;

public:
	NamedType(TypeContext& ctx, std::string name) :
		Type(ctx), name(name) { }

public:
	virtual size_t getBitSize() override { return getResolvedType()->getBitSize(); }
	virtual Type* getResolvedType() override { return ctx.resolveNamedType(name); };
	virtual std::string toString() override { return getResolvedType()->toString(); };
	virtual std::string toCppString() override { return getResolvedType()->toCppString(); };
	virtual bool isSame(Type* other) override { return Type::areSame(getResolvedType(), other); };

	virtual bool isVoidType() { return getResolvedType()->isVoidType(); }
	virtual bool isBoolType() { return getResolvedType()->isBoolType(); }
	virtual bool isIntegerType() { return getResolvedType()->isIntegerType(); }
	virtual bool isStructType() { return getResolvedType()->isStructType(); }
	virtual bool isPointerType() { return getResolvedType()->isPointerType(); }
	virtual bool isArrayType() { return getResolvedType()->isArrayType(); }
	virtual bool isCharType() { return getResolvedType()->isCharType(); }
	virtual bool isStringType() { return getResolvedType()->isStringType(); }
	virtual bool isSigned() { return getResolvedType()->isSigned(); }
};