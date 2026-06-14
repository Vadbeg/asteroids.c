# Learning Notes — Q&A Index

Topics covered, in order. Search this when something feels familiar.

## How C works
- Compilation pipeline: preprocess → compile → assemble → link.
- Source → translation unit → assembly → object file (`.o`) → executable.
- Executable formats: Mach-O (macOS), ELF (Linux), PE (Windows).
- OS load: kernel maps segments, hands off to dynamic linker (`dyld`), which loads `.dylib`s, runs CRT `_start`, finally calls `main`.
- C is system-dependent on three axes: ISA (arm64/x86-64), OS format + syscall ABI, calling convention/ABI. Source is portable, binary is not.

## stdio.h and libc
- `stdio.h` declared by Apple on macOS because libc is per-platform (Apple's libSystem, glibc, musl, UCRT, …).
- C standard defines the *interface*; each OS vendor ships the *implementation* because syscalls differ per kernel.
- Headers live in the SDK; compiled code in `libSystem.B.dylib` (macOS).

## Hello world + main
- `int main(int argc, char **argv)`: `argc` = count incl. program name, `argv` = array of C strings, `argv[argc] == NULL`.
- `printf` is variadic; first arg MUST be a `const char *` format string. `%d`, `%s`, `%f`, `%p`, `%zu`, `%c`, `%x`, `%u`.
- Exit codes: 0 = success, non-zero = failure. Conventions: 2 = bad usage, 126 = not executable, 127 = not found, 128+N = killed by signal N. `$?` in shell reads last exit code.

## Pointers
- Variable = a region of memory at an address. Pointer = variable holding an address. `&x` takes address, `*p` dereferences.
- `int *`, `int **`, `int ***` are distinct types. `*` in declaration BUILDS the type; `*` in expression TEARS DOWN indirection.
- "Declaration mirrors use" — `int **pp` means `**pp` yields an `int`.
- Parameters are pass-by-value, always. To mutate caller's data, pass an address. `argv` is `char **` because it's an address of an array of `char *`.

## typedef and structs
- `typedef` is a compile-time *alias* — no runtime cost, doesn't create a new type.
- C has separate namespaces: struct tags vs ordinary identifiers. That's why `struct Vec2` vs `Vec2` exists.
- Canonical form for projects: `typedef struct Vec2 { ... } Vec2;` — gets both tag and ordinary name.
- Anonymous struct + typedef works because typedef takes any type expression; the struct has no tag-namespace name but `Vec2` lives in the ordinary namespace.
- **Name equivalence**: two structs with identical fields but different definitions are NOT interchangeable. Each `struct { ... }` body creates a new type identity.
- Self-reference requires the tag: `struct Vec5 *next;` inside the body (typedef name not yet defined).

## Struct memory layout
- Fields laid out contiguously, in declaration order, with **padding** for alignment.
- Field offset = multiple of its own alignment. Struct alignment = max of field alignments. Total size padded to a multiple of struct alignment (so arrays stay aligned).
- `sizeof(Ship)` = 24, not 21, because of 3 bytes trailing padding.
- Field order matters: declare largest-alignment fields first to minimize padding.
- Inspect with `sizeof`, `offsetof(T, field)` (`<stddef.h>`), `_Alignof(T)`.

## Functions, value vs pointer
- Small struct → pass by value is fine (`Vec2`).
- Large struct → pass `T *` to avoid copying.
- `(*p).x` ≡ `p->x`. Always use `->` on pointers.

## Arrays
- An array is a contiguous block of bytes. No length stored, no bounds checking. `xs[i]` is `*(xs + i)`.
- **Array decays to pointer** in most expressions. NOT in `sizeof(xs)`, `&xs`, or string-literal initializers.
- Inside a function, `void f(int xs[5])`, `void f(int xs[])`, `void f(int *xs)` are identical. Length info is lost — pass it separately as `size_t n`.
- Idiom: `for (size_t i = 0; i < n; i++) { ... }`. `size_t` is unsigned, defined in `<stddef.h>`, returned by `sizeof`.
- Fixed-size pool with `alive` flag is the standard pattern for game entities; avoids per-frame `malloc`/`free`.

## stdbool.h
- C had no `bool` until C99. Codebases rolled their own (`typedef int bool;`).
- C99 added real type `_Bool` (ugly name to avoid collision) + `<stdbool.h>` shim defining `bool`, `true`, `false` as macros.
- Header, not a library — pure preprocessor text. Same opt-in pattern as `<stdint.h>`, `<stddef.h>`, `<stdalign.h>`.
- C23 made `bool` a real keyword; header still included for compatibility.

## .h vs .c files
- Compiler processes one translation unit at a time. `main.c` doesn't see `ship.c` — it sees `ship.h` via `#include`.
- **Declaration**: name + signature, no body. Can repeat. Goes in `.h`.
- **Definition**: actual body/storage. Must be unique program-wide (One Definition Rule). Goes in `.c`.
- `#include` is textual paste by the preprocessor, not a module import.
- **Header guards** mandatory: `#ifndef FOO_H / #define FOO_H / ... / #endif`. Or `#pragma once`.
- `static` at file scope = module-private (linker can't see it).
- Compile errors → missing include/typo. Linker errors → undefined symbol (missing `.o`) or duplicate symbol (function body in header / defined twice).
- Build: `clang -c file.c -o file.o` per file, then link all `.o`s.
