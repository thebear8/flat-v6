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

	virtual ~Type() { }

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
	size_t m_pointerSize;
	std::unordered_map<size_t, IntegerType*> m_signedIntegerTypes;
	std::unordered_map<size_t, IntegerType*> m_unsignedIntegerTypes;
	std::unordered_map<Type*, PointerType*> m_pointerTypes;
	std::unordered_map<Type*, ArrayType*> m_arrayTypes;
	/*
	std::unordered_map<std::string, StructType*> structTypes;
	std::unordered_map<std::string, Type*> builtinTypes;
	*/

	VoidType* m_void;
	BoolType* m_bool;
	IntegerType* m_i8;
	IntegerType* m_i16;
	IntegerType* m_i32;
	IntegerType* m_i64;
	IntegerType* m_u8;
	IntegerType* m_u16;
	IntegerType* m_u32;
	IntegerType* m_u64;
	CharType* m_char;
	StringType* m_string;

public:
	TypeContext();
	~TypeContext();

public:
	size_t getPointerSize() { return m_pointerSize; }
	size_t setPointerSize(size_t size) { return (m_pointerSize = size); }

	VoidType* getVoid() { return m_void; }
	BoolType* getBool() { return m_bool; }
	IntegerType* getI8() { return m_i8; }
	IntegerType* getU8() { return m_u8; }
	IntegerType* getI16() { return m_i16; }
	IntegerType* getU16() { return m_u16; }
	IntegerType* getI32() { return m_i32; }
	IntegerType* getU32() { return m_u32; }
	IntegerType* getI64() { return m_i64; }
	IntegerType* getU64() { return m_u64; }
	CharType* getChar() { return m_char; }
	StringType* getString() { return m_string; }

	IntegerType* getIntegerType(size_t width, bool isSigned);
	PointerType* getPointerType(Type* base);
	ArrayType* getArrayType(Type* base);
	
	/*
	StructType* getStructType(std::string const& name);
	Type* getResolvedType(std::string const& name);
	*/
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
		Type(ctx), bitSize(bitSize) { }

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

	void addField(std::string const& fieldName, Type* fieldType) { return fields.push_back(std::pair(fieldName, fieldType)); }
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