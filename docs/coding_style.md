Coding style for this library
-----------------------------

Naming extenions
----------------
- Typenames should like glsl for familiar: vec2, ivec2, mat4, ...
- SIMD should be __m<size> follow the SSE for all platforms, we have sse_to_neon.h as a wrapper for ARM target machine.
- Variable names should be snake_case
- Function names should be snake_case with module name (normally are typenames): vec2_new, mat4_mul, ...
- Function names should be describe what they do, add params type names for multi version: vec3_add_vec2, mat4_mul_vec3, ...
- Preprocessor names are ALL_UPPER_CASES, specially are `constexpr` and `__forceinline`
- File names are snake_case, because each file is a module
- Special case is standard math library extensions, which should be follow the standard style
- Testing no need to follow the rules.

Function implementation
-----------------------
- Avoid branching!
- Donot add more SIMD implementation module, instead add a wrapper, like sse_to_neon.h do!
- Support both C and C++, and function overloading in C++.
- Only add basic operator overloading (+,-,*,/), with SIMD meaning.
- Prefer fast with simple implementation over fastest with complex.
- The special cases will have their docs to describe the design decision of each implementations.
- Parameters are immutable.
- This is fixed-size vector and matrix library for graphics programming, so do not add pointer parameters to keep it simple and ease to use. There will be special case, otherwise. But now, we dont have one.