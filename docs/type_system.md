# flat-v6 Type System

## General Aspects
- Types belong to TypeContext
- Types are handled as pointers
- Pointers are guaranteed to be constant
- Comparison is simple, just compare pointers

## Concrete Types

- ## VoidType
    - Undefined width
- ## BoolType
    - 1 bit
- ## IntegerType
    - 8, 16, 32 or 64 bit
- ## CharType
    - 32 bit
- ## StringType
    - Array of u8
- ## StructType
    - Collection of field names and types
- ## PointerType
    - Contains underlying base type
    - Guaranteed to be unique for every base type
- ## ArrayType
    - Contains underlying base type
    - Guaranteed to be unique for every base type