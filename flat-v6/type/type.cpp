#include "type.hpp"

TypeContext::TypeContext()
    : m_pointerSize(),
      m_void(new VoidType(*this)),
      m_bool(new BoolType(*this)),
      m_i8(new IntegerType(*this, true, 8)),
      m_i16(new IntegerType(*this, true, 16)),
      m_i32(new IntegerType(*this, true, 32)),
      m_i64(new IntegerType(*this, true, 64)),
      m_u8(new IntegerType(*this, false, 8)),
      m_u16(new IntegerType(*this, false, 16)),
      m_u32(new IntegerType(*this, false, 32)),
      m_u64(new IntegerType(*this, false, 64)),
      m_char(new CharType(*this, 32)),
      m_string(new StringType(*this))
{
    m_signedIntegerTypes.try_emplace(8, m_i8);
    m_signedIntegerTypes.try_emplace(16, m_i16);
    m_signedIntegerTypes.try_emplace(32, m_i32);
    m_signedIntegerTypes.try_emplace(64, m_i64);
    m_unsignedIntegerTypes.try_emplace(8, m_u8);
    m_unsignedIntegerTypes.try_emplace(16, m_u16);
    m_unsignedIntegerTypes.try_emplace(32, m_u32);
    m_unsignedIntegerTypes.try_emplace(64, m_u64);

    /*
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
    */
}

TypeContext::~TypeContext()
{
    delete m_void;
    delete m_bool;
    delete m_char;
    delete m_string;

    for (auto& [base, type] : m_pointerTypes)
        delete type;
    for (auto& [base, type] : m_arrayTypes)
        delete type;
    for (auto& [width, type] : m_signedIntegerTypes)
        delete type;
    for (auto& [width, type] : m_unsignedIntegerTypes)
        delete type;

    /*
    for (auto& [name, type] : structTypes)
        delete type;
    for (auto& [name, type] : builtinTypes)
        delete type;
    */
}

IntegerType* TypeContext::getIntegerType(size_t width, bool isSigned)
{
    if (isSigned)
    {
        if (!m_signedIntegerTypes.contains(width))
            m_signedIntegerTypes.try_emplace(
                width, new IntegerType(*this, true, width));
        return m_signedIntegerTypes.at(width);
    }
    else
    {
        if (!m_unsignedIntegerTypes.contains(width))
            m_unsignedIntegerTypes.try_emplace(
                width, new IntegerType(*this, false, width));
        return m_unsignedIntegerTypes.at(width);
    }
}

PointerType* TypeContext::getPointerType(Type* base)
{
    if (!m_pointerTypes.contains(base))
        m_pointerTypes.try_emplace(base, new PointerType(base));
    return m_pointerTypes.at(base);
}

ArrayType* TypeContext::getArrayType(Type* base)
{
    if (!m_arrayTypes.contains(base))
        m_arrayTypes.try_emplace(base, new ArrayType(base));
    return m_arrayTypes.at(base);
}

/*
StructType* TypeContext::getStructType(std::string const& name)
{
    if (!structTypes.contains(name))
        structTypes.try_emplace(name, new StructType(*this, name, {}));
    return structTypes.at(name);
}
*/

/*
Type* TypeContext::getResolvedType(std::string const& name)
{
    if (builtinTypes.contains(name))
        return builtinTypes.at(name);
    else if (structTypes.contains(name))
        return structTypes.at(name);
    return nullptr;
}
*/

size_t StructType::getBitSize()
{
    size_t bitSize = 0;
    for (auto& [fieldName, fieldType] : fields)
    {
        bitSize += fieldType->getBitSize();
        bitSize =
            (size_t)(ceil(bitSize / (double)ctx.getPointerSize()) * ctx.getPointerSize());  // align bitSize to pointer size
    }

    return bitSize;
}