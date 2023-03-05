# Constraints
## Constraint templates
- Can take type params
- Contain a list of functions with parameters and result type

## Problems
- How do we deal with partial specialization?
- Example:
```flat
fn logTwoValuesWithPrefix<TPrefix, T1, T2>(prefix: TPrefix, a: T1, b: T2) {
    ...
}

constraint Bla<T> {
    logTwoValues<TPrefix>(prefix: TPrefix, a: T, b: i32) // what to do?
}

fn doSomething<T>(a: T, b: i32) [Bla<T>] {
    logTwoValues(123, a, b) // what to do?
}
```
- Result: Constraint conditions cannot have type parameters, they have to be fully specified
- You can however add more type parameters to the enclosing constraint:
```flat
fn logTwoValuesWithPrefix<TPrefix, T1, T2>(prefix: TPrefix, a: T1, b: T2) {
    ...
}

constraint Bla<T, TPrefix> {
    logTwoValues(prefix: TPrefix, a: T, b: i32)
}

fn doSomething<T>(a: T, b: i32) [Bla<T, i32>] {
    logTwoValues(123, a, b)
}
```