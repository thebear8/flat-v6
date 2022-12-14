#pragma once
#include <string>
#include <unordered_map>

class Type;
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
    virtual ~Type() {}

public:
    virtual size_t getBitSize()
    {
        throw std::exception("getBitSize() called on type that has no size");
    };

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

class VoidType : public Type
{
    virtual std::string toString() override { return "void"; };
    virtual bool isVoidType() override { return true; }
};

class BoolType : public Type
{
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
    IntegerType(bool signedness, size_t bitSize)
        : signedness(signedness), bitSize(bitSize)
    {
    }

public:
    virtual size_t getBitSize() override { return bitSize; };

    virtual std::string toString() override
    {
        return (signedness ? "i" : "u") + std::to_string(bitSize);
    };

    virtual bool isIntegerType() override { return true; }
    virtual bool isSigned() override { return signedness; }
};

class CharType : public Type
{
    virtual size_t getBitSize() override { return 32; };
    virtual std::string toString() override { return "char"; };
    virtual bool isCharType() override { return true; }
};

class StringType : public Type
{
    virtual std::string toString() override { return "str"; };
    virtual bool isStringType() override { return true; }
};

class StructType : public Type
{
public:
    std::string name;
    std::vector<std::pair<std::string, Type*>> fields;

public:
    StructType(
        std::string const& name,
        std::vector<std::pair<std::string, Type*>> const& fields)
        : name(name), fields(fields)
    {
    }

public:
    virtual std::string toString() override { return name; };
    virtual bool isStructType() override { return true; }
};

class PointerType : public Type
{
public:
    Type* base;

public:
    PointerType(Type* base) : base(base) {}

public:
    virtual std::string toString() override { return base->toString() + "*"; };
    virtual bool isPointerType() override { return true; }
};

class ArrayType : public Type
{
public:
    Type* base;

public:
    ArrayType(Type* base) : base(base) {}

public:
    virtual std::string toString() override { return base->toString() + "[]"; };
    virtual bool isArrayType() override { return true; }
};