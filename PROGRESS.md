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
- [X] Touch the out-of-bounds case briefly to feel UB — write `arr[7]` on a 5-element array, compile with `-fsanitize=address` to see the trap fire, then remove.

## In progress
- [ ] **Drawing in the console with ncurses** — chose ncurses over raw ANSI for simpler ergonomics. Three milestones:
  - [ ] **Milestone A — Static draw**: `initscr`, `cbreak`, `noecho`, `curs_set(0)`. Put a single `@` at fixed `(x, y)` with `mvaddch`. `refresh()`. Wait for any keypress (`getch`). `endwin()` on exit. Learn what happens if you forget `endwin` (terminal stays broken until `reset`).
  - [ ] **Milestone B — Game loop with movement**: non-blocking input (`nodelay(stdscr, TRUE)`, `keypad(stdscr, TRUE)`). Loop: `clear → read input → update `(x,y)` → draw → refresh → sleep ~16ms`. Arrow keys (or wasd) move the dot. `q` quits. This is the canonical game loop shape.
  - [ ] **Milestone C — Visualize the existing array**: replace the single dot with the `Vec2 positions[N]` array from the arrays exercise. Call existing `update(positions, velocities, N, dt)` each frame. Draw each as `*`. Watch the simulation actually drift on screen.

## Next up (in order)
1. Real `dt` via `clock_gettime(CLOCK_MONOTONIC, ...)` — replace the hardcoded `0.1f` with measured frame time. Step 5 of original plan; falls out naturally once the loop exists.
2. Wraparound: when a position goes off-screen, modulo it back. One line of math; visible immediately.
3. Signal handler for clean teardown on `Ctrl-C` (call `endwin` in handler) — avoids "hosed terminal" after killing mid-loop.
4. Refactor the array loop to use a proper `Asteroid` struct array with an `alive` flag — object-pool pattern. Worth doing once spawning/destruction enters the picture.
5. Split into multiple files: `vec2.{c,h}`, `ship.{c,h}`, `main.c`. Header guards. Defer until `main.c` is uncomfortable to navigate (~300 lines).
6. Write a tiny Makefile.
7. Ship physics: angle, thrust, vel decay (or no — Asteroids classically has no friction), wrap-around.
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
