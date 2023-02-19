# How to implement generics
## IRPass
- For every declaration
    - When entering scope, add type params to environment as generic

## SemanticPass
- Struct expressions
    - For every field:
        - Check if a matching field exists in the struct type
        - If the field in the struct type is generic
            - Check if the generic param can be deduced from the given value
- Function calls
    - Query functions by name first
    - For every found function
        - For every parameter
            - Check if parameter matches with arg
            - If parameter is generic
                - Check if type parameter for parameter can be deduced from arg

## GenericLoweringPass
- Transform all instantiations of generic functions
- For every instantiation of a generic function
    - Replace all instances of a generic type with the specified concrete type

```flat
struct GenericStruct<T> {
    a: vec<T>
}

let f = GenericStruct {
    a: getVecI32(), // a: vec<i32> => T: i32
}

```

```
function deduceTypeParams(concreteType, typeArgsForConcreteType, genericType, typeParamsForGenericType)
{
    if(genericType in typeParamsForGenericType)
    {
        yield (genericType, concreteType)
    }
    else
    {
        for each (contConcreteType, contGenericType) in (containedTypes(concreteType), containedTypes(genericType))
            yield deduceTypeParams(contConcreteType, typeArgsForConcreteType, contGenericType, typeParamsForGenericType)
    }
}
```