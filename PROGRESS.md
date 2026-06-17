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
- [x] **Arrays exercise**: `positions[N]` + `velocities[N]`, `update(positions, velocities, n, dt)`, tick loop, watched values drift linearly. Caught the `current_time` vs `dt` bug (quadratic drift) and fixed.
- [x] Random floats: `arc4random()` + cast-and-divide pattern, scaled to `[lo, hi)` via `random_float`.
- [x] `#define` vs `enum` vs `const int` for compile-time constants. Felt the no-semicolon rule.
- [x] Array decay rule generalized: about *context*, not just function calls. Three no-decay exceptions internalized.
- [x] Touch the out-of-bounds case briefly to feel UB — write `arr[7]` on a 5-element array, compile with `-fsanitize=address` to see the trap fire, then remove.
- [x] **Try ncurses** — got a feel for the library: `initscr`/`endwin`, the buffer model (`clear` → draw → `refresh`), `mvprintw`, `usleep`-driven animation loop, linking with `-lncurses`, Makefile target. Enough to understand its limits firsthand: character-cell resolution, no rotation, snap-to-grid movement. Decided not to push further into ncurses (Milestones B/C dropped).

## Pivot
Console rendering can't get close enough to original Asteroids (no smooth rotation, coarse grid, no real graphics). Switching to **raylib** for rendering: still C, single library, simple API, gives real 2D pixels, rotation, color, audio. The C-language learning goals on the "Next up" list are unchanged — only the rendering layer swaps. ncurses experiment stays in the repo as a reference for the linking/Makefile/game-loop shape.

## In progress
- [ ] **Install raylib and run the example** — `brew install raylib`, write the canonical "open a window, draw a circle, close on ESC" program from the raylib examples. Confirm linking with `-lraylib` works (plus any macOS framework flags raylib needs). Add a `raylib` target to the Makefile mirroring the `ncurses` one.

## Next up (in order)
1. Replace the example with our own loop: open window, draw a single `@`-equivalent (a small filled circle) at `(x, y)`, move it with arrow keys, `ESC`/`q` to quit. The raylib version of Milestone B.
2. Real `dt` — raylib gives `GetFrameTime()` for free; use it instead of a hardcoded step. Compare to `clock_gettime` mentally so the underlying idea is clear.
3. Port the `Vec2 positions[N]` + `velocities[N]` array sim onto the raylib window. Draw each as a small circle. Watch the drift.
4. Wraparound: positions wrap at window edges.
5. Refactor into an `Asteroid` struct array with an `alive` flag — object-pool pattern.
6. Split into multiple files: `vec2.{c,h}`, `ship.{c,h}`, `main.c`. Header guards. Defer until `main.c` is uncomfortable to navigate (~300 lines).
7. Ship: triangle drawn with rotation (angle as float, `DrawTriangle` or `DrawPoly` with rotation), thrust, vel decay (or none — classic Asteroids), wrap-around.
8. Asteroids: spawn, move, wrap, split when shot.
9. Bullets: pool, lifetime, fire on key.
10. Collision detection (circle-circle, distance squared).
11. Score, lives, game-over.

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
