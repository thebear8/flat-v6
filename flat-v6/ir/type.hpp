#pragma once
#include <string>
#include <vector>

#include "ir_node.hpp"

struct IRType : public IRNode
{
    virtual size_t getBitSize()
    {
        throw std::exception("getBitSize() called on type that has no size");
    };

    virtual std::string toString() = 0;

    virtual bool isGenericType() { return false; }
    virtual bool isVoidType() { return false; }
    virtual bool isBoolType() { return false; }
    virtual bool isIntegerType() { return false; }
    virtual bool isCharType() { return false; }
    virtual bool isStringType() { return false; }
    virtual bool isStructType() { return false; }
    virtual bool isPointerType() { return false; }
    virtual bool isArrayType() { return false; }
    virtual bool isSigned() { return false; }

    IMPLEMENT_ACCEPT()
};

struct IRGenericType : public IRType
{
    std::string name;

    IRGenericType(std::string const& name) : name(name) {}

    virtual std::string toString() override { return name; }
    virtual bool isGenericType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRVoidType : public IRType
{
    virtual std::string toString() override { return "void"; }
    virtual bool isVoidType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRBoolType : public IRType
{
    virtual size_t getBitSize() override { return 1; };
    virtual std::string toString() override { return "bool"; };
    virtual bool isBoolType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRIntegerType : public IRType
{
    bool signedness;
    size_t bitSize;

    IRIntegerType(bool signedness, size_t bitSize)
        : signedness(signedness), bitSize(bitSize)
    {
    }

    virtual size_t getBitSize() override { return bitSize; };

    virtual std::string toString() override
    {
        return (signedness ? "i" : "u") + std::to_string(bitSize);
    };

    virtual bool isIntegerType() override { return true; }
    virtual bool isSigned() override { return signedness; }

    IMPLEMENT_ACCEPT()
};

struct IRCharType : public IRType
{
    virtual size_t getBitSize() override { return 32; };
    virtual std::string toString() override { return "char"; };
    virtual bool isCharType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRStringType : public IRType
{
    virtual std::string toString() override { return "str"; }
    virtual bool isStringType() override { return true; }
};

struct IRStructType : public IRType
{
    std::string name;
    std::vector<IRGenericType*> typeParams;
    std::unordered_map<std::string, IRType*> fields;

    IRStructType(
        std::string const& name,
        std::vector<IRGenericType*> const& typeParams,
        std::unordered_map<std::string, IRType*> const& fields
    )
        : name(name), typeParams(typeParams), fields(fields)
    {
    }

    virtual std::string toString() override { return name; };
    virtual bool isStructType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRPointerType : public IRType
{
    IRType* base;

    IRPointerType(IRType* base) : base(base) {}

    virtual std::string toString() override { return base->toString() + "*"; };
    virtual bool isPointerType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRArrayType : public IRType
{
    IRType* base;

    IRArrayType(IRType* base) : base(base) {}

    virtual std::string toString() override { return base->toString() + "[]"; };
    virtual bool isArrayType() override { return true; }

    IMPLEMENT_ACCEPT()
};