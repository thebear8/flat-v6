# FLAT
A data oriented, simple to use programming language using LLVM as a backend.

Flat is currently still under heavy development, so things will change in the future.

## Syntax
### Structs
```
struct vec3i {
    x: i32,
    y: i32,
    z: i32,
}
```

### Functions
```
fn dotProduct(a: vec3i, b: vec3i): vec3i {
    return a.x * b.x + a.y * b.y + a.z * b.z
}
```

### Variables
```
let a = 3i32
let b = vec3 {
    x: 1,
    y: 2,
    z: 3,
}
```

### Modules and Imports
```
module std.math // This module is called std.math

import std.math.vector // Pull entities from std.math.vector into the scope of std.math
```

