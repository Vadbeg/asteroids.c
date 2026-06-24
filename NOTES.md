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
- **Whole-struct vs decomposed params is a design choice.** Passing `Asteroid *a` is cohesive, convenient, and doesn't churn when the struct grows — but couples the function to that type. Passing the minimal fields (`Vec2 *pos, Vec2 *vel`) keeps the function a *reusable primitive* (same `update`/`wrap` math works for bullets/ship) and narrows the contract (a fn taking `Vec2 *velocity` visibly cannot touch position). Pattern: keep low-level primitives on minimal types, add a thin per-entity wrapper (`update_asteroid(Asteroid*, ...)`) that calls them. Best of both.

## Copy vs. pointer to an array element
- `Asteroid a = arr[i];` **copies** the struct (structs assign by value). Mutating `a` then throws the change away — the array is untouched. Classic "updating a throwaway copy" bug; same family as returning a local.
- To mutate the real element: `Asteroid *a = &arr[i];` then `a->field`. Or address a field directly: `&arr[i].position` (`.` binds tighter than `&`, so parens optional).

## Compound literals (C99)
- Build an anonymous struct value inline, in an expression: `(Vec2){ ... }` — type name in parens + brace initializer. NOT `Vec2(...)` — a type is never callable in C (no constructors). That's a C++/Python/Rust habit.
- Works with designated init too: `(Vec2){.x = 1.0f, .y = 2.0f}`.
- Trailing comma: legal **inside** a brace initializer list (`{1, 2,}`), but a syntax error in a **function argument list** (`f(a, b,)`).

## float ↔ int conversions (silent traps)
- `float → int` assignment **truncates toward zero**, silently. Storing `position->x + velocity->x*dt` (e.g. `200.016`) into an `int` drops the fraction → with sub-pixel-per-frame motion the asteroid never moves at all. Keep positions `float` so tiny increments accumulate.
- In comparisons like `float >= int`, the `int` is promoted to `float` (usual arithmetic conversions) — works, but a quiet type mismatch. `-Wconversion` flags these; `-Wall -Wextra` does NOT.

## Positional arguments + implicit conversion (reorder bug)
- C matches arguments to parameters **purely by position, never by name**. Insert a new param in the *middle* of a signature and every call site silently shifts.
- Because `int`/`float` are implicitly convertible, a reordered call (passing `dt` where `int radius` is expected and `high=900` where `float dt` is expected) **compiles clean** and just behaves insanely (asteroids moved `velocity*900`/frame → flew off instantly). `-Wall -Wextra` won't catch it.
- Defenses: append new params at the END; bundle related data into one struct pointer; double-check call site vs definition order.

## Arrays
- An array is a contiguous block of bytes. No length stored, no bounds checking. `xs[i]` is `*(xs + i)`.
- An array name is NOT a pointer. It's the storage itself. In *most* expression contexts it **decays** to a pointer to its first element. Decay is a context rule, not a function-call rule.
- Three exceptions where decay does NOT happen: operand of `sizeof`, operand of `&`, string literal as initializer.
- `sizeof(xs)` inside the defining scope = total bytes (`N * sizeof(element)`). Inside a function that received the decayed array = `sizeof(pointer)` = 8. Easy way to feel decay.
- `&xs` has type `T (*)[N]` — pointer to whole array. Same numerical address as `xs`, different type → different pointer-arithmetic step (`(&xs)+1` jumps `sizeof(xs)` bytes).
- Inside a function, `void f(int xs[5])`, `void f(int xs[])`, `void f(int *xs)` are identical. Length info is lost — pass it separately as `size_t n`. The `5` is documentation only, completely ignored by the compiler.
- Idiom: `for (size_t i = 0; i < n; i++) { ... }`. `size_t` is unsigned, defined in `<stddef.h>`, returned by `sizeof`.
- `E1[E2]` ≡ `*((E1) + (E2))`. Therefore `xs[i]` ≡ `i[xs]` (commutative). And `&xs[i]` ≡ `xs + i`.
- `xs[i]` is an **lvalue** — a named location. Reading it gives the value; assigning to it writes to the original storage. That's why `xs[i] = 40` works without explicit pointer syntax.
- Fixed-size pool with `alive` flag is the standard pattern for game entities; avoids per-frame `malloc`/`free`.
- **OOB writes**: no language-level bounds check. Write lands on whatever happens to be at that address — adjacent locals, padding, return address (stack smashing), heap bookkeeping, or unmapped page → SIGSEGV. UB means the compiler may also optimize away the surrounding code. Catch in dev with `-fsanitize=address`.

## Pass-by-value invariant
- C *always* passes parameters by value. Always. There is no pass-by-reference.
- "Arrays aren't copied" is true but misleading: it's because the array decays to a pointer *at the call site*, and the pointer (8 bytes) is what's copied. The 40 bytes of contents are shared, not copied.
- To pass an array by value, wrap it in a struct: `struct Wrapper { int xs[5]; };` — structs do pass by value.
- Mutation through `T *` works because the address is copied, but the address still refers to the original storage. Same mechanism whether `T *` means "pointer to one" or "pointer to first of many" — the language doesn't distinguish; intent is yours.

## #define and the preprocessor
- Preprocessor runs *before* the compiler. Pure textual substitution. By compile time, every `#define` has been expanded away — there is no `N` symbol in the binary, just the literal it was replaced with.
- `#define` is a preprocessor line, not a C statement. Ends at the newline, **no semicolon**. `#define N 5;` makes the macro body `5;`, which then breaks every use site (`positions[5;]` is a syntax error).
- Function-like macros: `#define SQR(x) ((x) * (x))`. Parenthesize aggressively — no scope, no type-check, just text paste. `DOUBLE(3 + 1)` without parens around `x` expands to `3 + 1 * 2 = 5`, not 8.
- Choices for compile-time integer constants:
  - `#define N 5` — preprocessor, no type, works anywhere a constant expression is needed (array sizes, `case` labels).
  - `enum { N = 5 };` — typed `int`, scoped, debugger-visible, also a constant expression. Often the cleanest.
  - `const int N = 5;` — in **C** this is NOT a constant expression! `int xs[N]` becomes a VLA. (Different from C++.)
- Header guards `#ifndef FOO_H / #define FOO_H` use this same machinery — `#define FOO_H` with no body, just to mark a name as "defined."

## VLAs
- Variable-Length Array: array on the stack whose size is a runtime value, not a constant expression. Introduced in C99, made *optional* in C11. MSVC doesn't support them. Linux kernel banned them in 2018.
- `int N = 5; int xs[N];` — VLA. The compiler emits `sub %rsp, N*sizeof(int)` at runtime to allocate.
- Why avoided: (1) stack overflow if `N` is large or attacker-controlled — no graceful failure, just SIGSEGV; (2) negative/zero size = UB; (3) `sizeof(xs)` becomes a runtime computation, not a constant; (4) optimizer-unfriendly; (5) portability.
- For fixed game pools, always use a compile-time constant via `#define` or `enum`. Heap-allocate (`malloc`) for genuinely runtime-sized buffers.

## Variadic functions and printf
- `printf` is variadic — the compiler cannot check that arg types match the format string. It just passes args according to the platform ABI.
- Floats passed through `...` are promoted to `double` and travel in floating-point registers. Ints travel in integer registers. **Different physical locations.**
- `printf("%d", a_float)` reads from the integer register expecting an `int` — gets garbage. Same UB regardless of how clean the numeric value is. Compile with `-Wall -Wextra` and clang catches this.
- `%p` expects `void *` specifically. Pass other pointer types only via explicit cast: `(void *)positions`.
- `%f` for `float`/`double`. `%zu` for `size_t`. `%d` for `int`. `%u` for `unsigned`.

## void * and type erasure
- `void *` = "pointer to anything." Holds an address, no type info about the pointee.
- Cannot dereference (`*p` makes no sense — as what type?) and cannot do pointer arithmetic (no step size).
- Any pointer can be assigned to `void *` and back without explicit cast (in C; C++ requires a cast).
- Used by `malloc` (returns `void *`), `memcpy`/`memset`, and `%p`. The C version of "generic reference."

## Randomness
- `rand()` + `srand()` is the standard-C answer but bad: implementation-defined quality, `RAND_MAX` may be as low as 32767, modulo bias on `rand() % N`, one-second seed resolution, not thread-safe.
- On macOS/BSD: `arc4random()` returns a `uint32_t`, kernel-seeded, no `srand` needed. `arc4random_uniform(N)` gives a bias-free integer in `[0, N)`.
- Build a `float` in `[0, 1)`: `(float)arc4random() / (float)UINT32_MAX`. Cast *before* division — both operands int would give integer division (always 0).
- Scale to `[lo, hi)`: `lo + unit * (hi - lo)`.
- Float literal `f` suffix matters: `1000.0` is `double`, `1000.0f` is `float`. Mixing triggers `-Wdouble-promotion`.

## Process isolation and memory safety
- Modern OS gives each process its own **virtual address space**, mapped to physical RAM by the MMU via per-process page tables.
- Same numerical address means *different physical memory* in different processes. You cannot reach another process's RAM by accident (or by raw OOB write).
- OOB writes can land on: own stack (adjacent locals, return address → stack smashing), own heap (allocator corruption), own globals, or unmapped pages (SIGSEGV). Code pages mapped read-only, so writes there trap immediately.
- Indirect blast radius beyond memory: files/network/IPC your process has permission for. If running as root, the system. Security exploits chain an OOB write → control-flow hijack inside the process.
- In-process defenses: ASLR, stack canaries (`-fstack-protector`), NX/DEP, CFI. Dev-time detection: `-fsanitize=address,undefined`.
- The mental model: "the OS contains your bugs to your process, but inside your process you are the runtime."

## stdbool.h
- C had no `bool` until C99. Codebases rolled their own (`typedef int bool;`).
- C99 added real type `_Bool` (ugly name to avoid collision) + `<stdbool.h>` shim defining `bool`, `true`, `false` as macros.
- Header, not a library — pure preprocessor text. Same opt-in pattern as `<stdint.h>`, `<stddef.h>`, `<stdalign.h>`.
- C23 made `bool` a real keyword; header still included for compatibility.

## Linking external libraries (ncurses, raylib)
- Header (`.h`) is declarations; library (`.dylib`/`.a`) is the machine code. `#include` alone isn't enough — also need `-l<name>` so the linker can resolve symbols.
- `-l<name>` → linker searches for `lib<name>.dylib` / `.so` / `.a` in standard library dirs.
- Apple's clang does NOT search `/opt/homebrew/{include,lib}` by default. For brew-installed libs, must pass `-I/opt/homebrew/include -L/opt/homebrew/lib` explicitly.
- Library flags go at the END of the compile line — left-to-right symbol resolution.
- macOS-specific: Apple system frameworks pulled in via `-framework Name` (raylib needs `CoreVideo IOKit Cocoa OpenGL`). Different mechanism from `-l`.
- Static vs dynamic linking: `.a` archive → code copied into binary; `.dylib` → only a recorded dependency, loaded at runtime by `dyld`. `otool -L ./bin` shows what a binary actually links against.
- Argument ordering only matters if your linker is traditional. Modern `lld` may be lenient — don't rely on it.
- Better than hardcoding paths: `pkg-config --cflags --libs <name>` (raylib ships a `.pc` file); for big projects, CMake/Meson.

## clangd vs clang
- `clangd` (VS Code's language server) is a separate process from `clang`. It doesn't read your Makefile.
- For squigglies/IntelliSense to work with non-default include paths, give clangd the same flags: `compile_flags.txt` (one flag per line) or `compile_commands.json` (per-file commands, auto-generated by `bear` or CMake).

## Preprocessor recap
- `#include` is a directive, NOT a statement. No semicolon. Line-based, not statement-based.
- `#include "x.h"` → search current dir first, then system. `#include <x.h>` → system only. Convention: quotes for own headers, angle brackets for libraries.

## Homebrew
- Package manager for macOS. Formulae describe install steps; bottles are prebuilt binaries.
- Apple Silicon prefix: `/opt/homebrew`. Files live in `Cellar/<formula>/<version>/`; symlinks point from `/opt/homebrew/{bin,lib,include,share}` into the Cellar.
- `brew list <formula>` to see installed files; `brew info <formula>` for metadata without installing; `brew --prefix <formula>` for the install dir.
- macOS often ships its own copy of common libs (ncurses, OpenSSL) — `brew list ncurses` may be empty because you're linking the system one. Man pages come from upstream projects, not brew or Apple.

## man pages
- Sections: 1 = user commands, 2 = syscalls, 3 = C library funcs, 5 = file formats, 7 = misc, 8 = sysadmin. `man 3 printf` forces the C lib version.
- `apropos <kw>` / `man -k <kw>` for search; `man -w <name>` shows the file path; `whatis <name>` for one-liners.
- Pager is `less`, not vim. Shares vi-style keys but is read-only. `PAGER` env var overrides.
- Man pages are written upstream by project authors in roff. Not every library has them — raylib doesn't; ncurses does (because GNU ncurses upstream wrote them).

## ncurses (touched, not mastered)
- Terminal-control library. `initscr` takes over, `endwin` releases (forget it → terminal stays broken until `reset`).
- Buffer model: `clear` / draw calls / `refresh` — nothing visible until `refresh` swaps the buffer to screen.
- `mvprintw(row, col, "...")` — row first, then column. Different from `(x, y)` mental model.
- For a real game loop would also need: `cbreak` (no line buffering), `noecho` (don't echo input), `curs_set(0)` (hide cursor), `nodelay` (non-blocking input), `keypad` (arrow key handling). Didn't implement — switched to raylib instead.

## raylib basics
- Single-header API (`<raylib.h>`), C99, zlib license. Init → loop → close pattern: `InitWindow(w, h, title)`, `while (!WindowShouldClose())`, `CloseWindow()`.
- Frame loop: exactly ONE `BeginDrawing` / `EndDrawing` pair per iteration.
  - `BeginDrawing` → bind framebuffer, reset batch.
  - Draw calls between them only append to an internal batch — not visible yet.
  - `EndDrawing` → flush batch, swap buffers (double buffering → no tearing), poll OS events, update `WindowShouldClose`, apply `SetTargetFPS` sleep.
- State updates conventionally go OUTSIDE the `BeginDrawing/EndDrawing` block — update phase, then draw phase.
- "Indent inside `BeginX`/`EndX`" is a visual convention to fake block scope for begin/end API pairs (`BeginMode2D`, `BeginShaderMode`, etc.). Not C syntax, just style.
- `SetTargetFPS(60)` + `GetFrameTime()` for free framerate-independent timing. Equivalent to `clock_gettime` work we'd otherwise do manually.
- Docs: header comments + raylib.com cheatsheet + examples folder. No man pages.

## raylib coordinates (screen space)
- Origin `(0,0)` is **top-left**. x grows right (same as math); **y grows DOWN** (opposite of math/plots). `(width, height)` is bottom-right.
- Consequence: **positive `velocity.y` moves DOWN.** Matters for ship thrust later, not for drifting asteroids.
- Units are floats in the API even though pixels are discrete; sub-pixel positions round at draw time.
- `DrawCircle(x, y, r, color)` treats `(x,y)` as the **center**, not a corner — so center can be in-bounds while body pokes past an edge.
- `GetScreenWidth()` / `GetScreenHeight()` give bounds at runtime — use instead of a hardcoded magic size so spawn/wrap stay correct.
- **Coordinate-wrapper temptation** (keep world as y-up, flip only at draw): legitimate pattern but premature here. Cost isn't the one-subtraction math — it's that *every* raylib call touching coords (input, mouse, text, textures) becomes a conversion site, plus carrying two systems in your head. Adopt the library's native convention until it demonstrably hurts; then a tiny `world_to_screen()` used only at draw calls beats a parallel universe. General C lesson: abstractions have carrying costs nobody amortizes for you — resist until the concrete version hurts.

## Thrust as vector addition (not scaling)
- Velocity is a **vector**; acceleration is also a vector. To speed up you **add** an acceleration vector to velocity — you do NOT multiply velocity by a scalar.
- Multiplication is the wrong tool for thrust: `0 × anything = 0`, so a ship at rest could never start; and a zero vector has no direction to scale toward. The "add a tiny nudge on the first frame" hack is a symptom of using the wrong operation.
- The thrust direction does **not** come from current velocity — it comes from the ship's heading (the angle). So thrust works fine from a dead stop: add a heading-direction vector and motion begins.
- Multiplication (scaling down) is the right tool only for **drag/friction** — shrinking an existing vector, where `0 × k = 0` (stopped stays stopped) is exactly what you want.
- Full thrust step: `unit_direction × acceleration_magnitude × dt`, added to velocity. Three independent knobs: direction (where), magnitude (how hard), `dt` (framerate independence).

## Angle representation (the unit war)
- Two facts that never change: `cosf`/`sinf` **always** need radians; raylib's `rotation` params **always** want degrees. No single stored unit satisfies both — you convert *somewhere* regardless.
- Decide by counting consumers. A **hand-built vector ship** (`DrawTriangle`/`DrawLineV` from vertices you compute via trig) has NO degree consumer at all → radians is native to everything. A `DrawPoly`/rotated-texture ship → that one call wants degrees.
- We chose to **store degrees** (easier to reason about / eyeball-debug: "90° is a quarter turn"). Cost: convert deg→rad (`* DEG2RAD`, a raylib macro) at every trig call. Accepted with eyes open.
- `get_direction(angle)` → unit vector `(cosf(rad), sinf(rad))`, magnitude 1 by the Pythagorean identity. Reused everywhere a heading direction is needed (thrust, nose marker, later bullets).
- Angle MUST be `float`, not `int`: rotation per frame is `angle_change * dt` = fractional (e.g. `50 * 0.016 ≈ 0.8`); an `int` truncates that to 0 and the ship never turns. Same `float→int` truncation trap as positions. Not `double` either — feeds `f`-trig and raylib's float API.
- Wrap the angle with `fmodf(a, 360.0f)` so it doesn't grow unbounded as you spin. Same modular idea as screen wraparound, on a scalar.
- **Screen y-down consequence:** `(cosf θ, sinf θ)` sweeps **clockwise** as θ increases (math convention is CCW, but y is flipped). Fine as long as the drawn nose and the thrust both come from the *same* angle through the *same* trig — pick what θ=0 means and stay consistent.

## Friction / drag models + natural terminal velocity
- Friction is a **separate, always-on** step: it runs every frame regardless of input (unlike thrust, which is gated on the key). Don't tangle it into the thrust function.
- **Exponential / viscous drag** (`velocity *= factor < 1`): force ∝ velocity. Simplest, can't reverse or overshoot, asymptotes toward 0. This is a *real* physical regime (slow motion through fluid), not a hack.
- **Constant / Coulomb friction** (subtract fixed step along `−velocity`): force constant. Linear slowdown, stops in finite time — but must **clamp at zero** or it overshoots and the ship creeps backwards.
- Original Asteroids is near-**frictionless** (spaceship in vacuum): you slow down by rotating and counter-thrusting, not drag. Adding drag is a design choice for feel.
- **Terminal velocity is a stable fixed point.** Per-frame recurrence (1D, constant thrust): `v_next = d·v + a·dt` — an affine recurrence (same shape as an EMA / discounted return). Fixed point `v* = a·dt / (1 − d)`, finite, no clamp needed.
- **Why it self-caps:** thrust adds a *constant* `a·dt` (doesn't grow with speed); drag removes `(1−d)·v` (*proportional* to speed). The proportional term inevitably catches the constant one — crossover = terminal velocity.
- **Why it's stable, not just balanced:** error `e = v − v*` obeys `e_next = d·e` → decays geometrically by `d` each frame from any start. `v*` is an attractor; approach is itself exponential. Converges iff `|d| < 1` (a contraction).
- **Why exponential specifically gives a free cap:** constant (Coulomb) friction makes the recurrence `v_next = v + a·dt − f` — both terms constant in `v`, no speed-dependent term → if `a·dt > f`, speed grows forever (no terminal velocity). Only a drag term that *scales with the quantity you want to limit* self-regulates. This is why the hard `ceiling` clamp became redundant once exponential drag was in.

## Framerate-independent damping
- A raw per-frame `velocity *= 0.99` is **framerate-dependent**: it fires 60×/s vs 144×/s, so the ship slows much faster on a fast monitor AND terminal velocity shifts (`v*` depends on the effective `d`). Invisible while `SetTargetFPS` locks `dt` — bites when framerate changes. (Known TODO in `apply_friction`: it takes `dt` but doesn't use it yet.)
- Fix: express drag as "fraction remaining **per second**" applied over `dt`. Stretch the factor across arbitrary `dt` by raising to the `dt` power: `friction^dt` (`powf`), or equivalently `e^(−k·dt)` (`expf`). These make two half-frames compose into one full frame; a plain multiply does not.

## Capping speed while keeping steering (magnitude clamp)
- Gating thrust with `if (speed < ceiling)` kills steering at top speed — at the cap, thrust is switched off entirely, so you can't even *redirect*.
- Right approach: always apply thrust, then **clamp the result's magnitude**. Set a vector to length L while preserving direction: `v × (L / current_length)` (normalize, then scale). At the cap, thrusting sideways rotates velocity toward the new heading without growing it → steering survives.
- Guard `length == 0` (division by zero) before dividing.
- In practice, once exponential drag gave a natural terminal velocity below `ceiling`, the explicit clamp was dropped entirely.

## Your Vec2 vs raylib's Vector2 (name equivalence, in practice)
- raylib's `Vector2` and your `Vec2` have identical layout (two floats) but are **different type identities** (the name-equivalence rule). The `…V` draw calls (`DrawLineV`, `DrawCircleV`) take `Vector2` and will reject your `Vec2`.
- Sidestep: use the **scalar-argument** draw functions (`DrawLine(x1,y1,x2,y2,...)`, `DrawCircle(x,y,r,...)`) — pass `.x`/`.y` out of your own struct, never touch raylib's type. (Or construct a `Vector2` at the call boundary if you want the `…V` forms.)

## Visualizing heading
- Reuse `get_direction(angle)` and project a point in front of the ship: `nose = center + direction × length`. Same `unit-vector × scalar + position` pattern as thrust, applied to position for rendering.
- A **line** from center to nose reads as "pointing" more clearly than a dot. Scale a bit past the radius (`× radius × 1.5`) so the nose pokes out. Stepping stone toward a real triangle ship (3 vertices from the angle).

## Wraparound (toroidal screen)
- Original Asteroids wraps everything (ship, rocks, bullets) — top↔bottom, left↔right. Not wall-bounce; the game has no walls.
- **Body-vs-center matters for smoothness.** Exit only when the whole body is gone (`center - radius >= high`), re-enter flush against the opposite edge (`center = 0 - radius`, leading edge just touching). Snapping the center instead causes a visible pop.
- Use `else if` for the two opposite edges — independent `if`s fire on the same value (wrap then immediately un-wrap).
- Wrap x against **width**, y against **height** — only equal while the window is square; don't conflate them.

## Collision detection (the core mental shift)
- **Never iterate pixels/outline.** Swap the render shape for a cheap math **proxy** — a bounding circle (center + radius) — and test with a formula.
- Two circles overlap iff distance between centers `< r1 + r2`. One comparison per pair, no loop over points.
- **Squared-distance trick:** compare `dx*dx + dy*dy < (r1+r2)*(r1+r2)` to skip `sqrt` (expensive). Valid because squaring preserves order for non-negatives.
- Tune the collision radius independently of the drawn shape (slightly smaller = fair near-misses).
- **Original Asteroids collision matrix:** asteroids do NOT collide with each other (would need real rigid-body physics + N² checks; rocks are just hazards drifting in straight lines). Only cross-type pairs: asteroid×{ship, bullet, saucer}. So your loops are always across categories, never self-paired — cheaper and simpler.

## Game loop shape (portable to web)
- Body of the loop should be a callable function: one frame = one call.
  - Native: `while (!WindowShouldClose()) update_and_draw_frame(&state);`
  - Web/Emscripten: `emscripten_set_main_loop_arg(update_and_draw_frame, &state, 0, 1);`
- Browser can't run `while(true)` — must yield to the event loop. Structuring loop body as a function from day one makes the eventual wasm/Emscripten build a recompile, not a refactor.

## .h vs .c files
- Compiler processes one translation unit at a time. `main.c` doesn't see `ship.c` — it sees `ship.h` via `#include`.
- **Declaration**: name + signature, no body. Can repeat. Goes in `.h`.
- **Definition**: actual body/storage. Must be unique program-wide (One Definition Rule). Goes in `.c`.
- `#include` is textual paste by the preprocessor, not a module import.
- **Header guards** mandatory: `#ifndef FOO_H / #define FOO_H / ... / #endif`. Or `#pragma once`.
- `static` at file scope = module-private (linker can't see it).
- Compile errors → missing include/typo. Linker errors → undefined symbol (missing `.o`) or duplicate symbol (function body in header / defined twice).
- Build: `clang -c file.c -o file.o` per file, then link all `.o`s.
