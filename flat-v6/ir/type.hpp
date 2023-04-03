#pragma once
#include <tsl/ordered_map.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "ir_node.hpp"

struct IRModule;

struct IRType : public IRNode
{
    virtual std::size_t getBitSize()
    {
        throw std::runtime_error("getBitSize() called on type that has no size"
        );
    };

    virtual std::string toString() = 0;

    virtual bool isGenericType() { return false; }
    virtual bool isVoidType() { return false; }
    virtual bool isBoolType() { return false; }
    virtual bool isIntegerType() { return false; }
    virtual bool isCharType() { return false; }
    virtual bool isStringType() { return false; }
    virtual bool isStructType() { return false; }
    virtual bool isStructTemplate() { return false; }
    virtual bool isStructInstantiation() { return false; }
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
    virtual std::size_t getBitSize() override { return 1; };
    virtual std::string toString() override { return "bool"; };
    virtual bool isBoolType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRIntegerType : public IRType
{
    bool signedness;
    std::size_t bitSize;

    IRIntegerType(bool signedness, std::size_t bitSize)
        : signedness(signedness), bitSize(bitSize)
    {
    }

    virtual std::size_t getBitSize() override { return bitSize; };

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
    virtual std::size_t getBitSize() override { return 32; };
    virtual std::string toString() override { return "char"; };
    virtual bool isCharType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRStringType : public IRType
{
    virtual std::string toString() override { return "str"; }
    virtual bool isStringType() override { return true; }
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

struct IRStruct : public IRType
{
    std::string name;
    tsl::ordered_map<std::string, IRType*> fields;

    IRStruct(
        std::string const& name,
        tsl::ordered_map<std::string, IRType*> const& fields
    )
        : name(name), fields(fields)
    {
    }

    virtual std::string toString() override { return name; };
    virtual bool isStructType() override { return true; }

    IMPLEMENT_ACCEPT()
};

struct IRStructTemplate : public IRStruct
{
    std::vector<IRGenericType*> typeParams;

    IRStructTemplate(
        std::string name,
        std::vector<IRGenericType*> const& typeParams,
        tsl::ordered_map<std::string, IRType*> const& fields
    )
        : IRStruct(name, fields), typeParams(typeParams)
    {
    }

    virtual std::string toString() override
    {
        std::string args = "";
        for (auto arg : typeParams)
            args += (!args.empty() ? ", " : "") + arg->toString();
        return IRStruct::toString() + "<" + args + ">";
    }

    virtual bool isStructTemplate() override { return true; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(parent, IRModule*, getParent, setParent)
};

struct IRStructInstantiation : public IRStruct
{
    std::vector<IRType*> typeArgs;

    IRStructInstantiation(
        std::string name,
        std::vector<IRType*> const& typeArgs,
        tsl::ordered_map<std::string, IRType*> const& fields
    )
        : IRStruct(name, fields), typeArgs(typeArgs)
    {
    }

    virtual std::string toString() override
    {
        std::string args = "";
        for (auto arg : typeArgs)
            args += (!args.empty() ? ", " : "") + arg->toString();
        return IRStruct::toString() + "<" + args + ">";
    }

    virtual bool isStructInstantiation() override { return true; }

    IMPLEMENT_ACCEPT()

    METADATA_PROP(
        instantiatedFrom,
        IRStructTemplate*,
        getInstantiatedFrom,
        setInstantiatedFrom
    )
};