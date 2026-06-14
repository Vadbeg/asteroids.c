# Progress

Project: console Asteroids in C, as a vehicle for learning the language.

## Setup
- macOS, clang + clangd via VS Code.
- Single-file project so far: `main.c`.
- Always compile with: `clang -Wall -Wextra -Wpedantic main.c -o main`

## Done
- [x] Hello World — printf, `\n`, exit codes.
- [x] `argc` / `argv` mechanics (didn't implement looping over argv yet, just understood).
- [x] Pointers: declare, take address, dereference, chain (`*p`, `**pp`).
- [x] Mutating a variable through a pointer chain — verified with `**pp = 99`.
- [x] Structs: define with `typedef struct Vec2 { ... } Vec2;`.
- [x] `vec2_add` — pass-by-value, returns new struct.
- [x] `vec2_add_in_place` — pass-by-pointer, mutates caller.
- [x] `sizeof(Vec2) = 8`, `sizeof(Ship) = 24` — observed padding firsthand.

## In progress
- [ ] **Arrays exercise**: `positions[N]` + `velocities[N]`, `update(positions, velocities, n, dt)`, `print_positions`, loop a few "ticks" and watch values drift.
- [ ] Touch the out-of-bounds case briefly to feel UB, then remove.

## Next up (in order)
1. Refactor the array loop to use a proper `Asteroid` struct array with an `alive` flag — object-pool pattern.
2. Split into multiple files: `vec2.{c,h}`, `ship.{c,h}`, `main.c`. Learn the build commands. Header guards.
3. Write a tiny Makefile.
4. Hello ncurses: put a `@` on screen, move with arrow keys, quit on `q`. Non-blocking input + frame loop + cleanup.
5. Game loop with `clock_gettime` and `delta_time`.
6. Ship physics: pos += vel*dt, angle, thrust, wrap-around at screen edges.
7. Asteroids: spawn, move, wrap, split when shot.
8. Bullets: pool, lifetime, fire on key.
9. Collision detection (circle-circle, distance squared).
10. Score, lives, game-over.

## Style decisions made
- Canonical struct form: `typedef struct Name { ... } Name;`.
- Float literals get the `f` suffix (`5.0f`).
- Designated initializers: `Vec2 v = {.x = 1.0f, .y = 2.0f};`.
- `size_t` for indices and lengths.
- `->` over `(*p).field`.
- Use `static` for module-private functions once we go multi-file.

## Mental model anchors
- C variables are addresses with types. No heap objects, no rebinding.
- Parameters are local copies. Mutate caller → pass an address.
- Each `.c` is compiled alone; headers carry declarations between them.
- No bounds checking, no GC, no runtime type info. You are the runtime.
