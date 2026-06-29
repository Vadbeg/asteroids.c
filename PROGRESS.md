# Progress

Project: console Asteroids in C, as a vehicle for learning the language.

## Setup
- macOS, clang + clangd via VS Code.
- Single-file project: `main.c` (the raylib prototype graduated from `raylib_test.c` into `main.c`; old code removed).
- Build via `make` (Makefile wraps the raylib flags) or directly: `clang -Wall -Wextra -Wpedantic main.c -o main -lraylib -I/opt/homebrew/include -L/opt/homebrew/lib`.

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
- [x] **Install raylib + first window** — `brew install raylib`; learned brew headers aren't on clang's default search path on macOS, so the Makefile target uses `-I/opt/homebrew/include -L/opt/homebrew/lib -lraylib` plus the Apple frameworks. Wrote `raylib_test.c` with the canonical `while (!WindowShouldClose())` loop, one `BeginDrawing`/`EndDrawing` pair per frame, state declared outside the loop, update phase separated from draw phase. clangd squigglies fixed by giving it the same flags (`compile_flags.txt` or equivalent).
- [x] **wAsteroids drifting on the raylib window** — `Asteroid` struct array, `initialize_asteroids` fills caller-owned array in place (void, not return-a-local), compound literals `(Vec2){...}`, symmetric `±` speed bounds via `static const float` so they go all directions. Real `dt` via `GetFrameTime()`. `DrawCircle` per asteroid.
- [x] **Wraparound** — toroidal, body-vs-center (exit when whole body off `center - r >= high`, re-enter flush on opposite edge), `else if` per axis pair, accounts for radius. Verified smooth on all four edges with a single asteroid.
- [x] **Bugs caught & understood** (see NOTES): copy-vs-pointer element (`a = arr[i]` copies), `float→int` truncation freezing sub-pixel motion, positional-argument reorder with silent implicit conversion (asteroids flew off at `velocity*900`).
- [x] **Design discussions** (no code): collision via bounding-circle proxy + squared distance + cross-type-only matrix; whole-struct vs decomposed params; premature coordinate-wrapper abstraction; raylib screen-space y-down coords.
- [x] **Ship rotation** — `float angle` (degrees), `LEFT`/`RIGHT` change it by `angle_change * dt` (framerate-independent). `get_direction(angle)` → unit vector via `DEG2RAD` + `cosf`/`sinf`.
- [x] **Vector thrust** — `KEY_UP` adds `direction × acceleration × dt` to velocity (add, not scale). Killed the old per-axis zero-clamp that forbade up/left motion.
- [x] **Friction + natural terminal velocity** — exponential drag (`velocity *= friction`) applied every frame. Derived the fixed point `v* = a·dt/(1−d)` and why proportional drag self-caps; confirmed the hard speed `ceiling` is redundant and removed it.
- [x] **Heading marker** — line from ship center to `center + direction × radius`, drawn with scalar `DrawLine` to dodge the `Vec2` vs raylib `Vector2` type-identity clash.
- [x] **Vector-outline asteroids** — 4 original-game rock shapes as `static const Vec2 rockN[]` (absolute vertices, verified against the delta chains). Bugs worked through: deltas-stored-as-absolutes, the 2D rotation formula (cross terms + θ=0 identity test), and rotate-then-translate order. See NOTES "Encoding vector shapes", "Model → world transform", "2D rotation formula".
- [x] **Shape lookup table** — `Shape { const Vec2 *rock; int size; }` + `static const Shape rock_shapes[]`; each `Asteroid` carries an `int shape` index set via `arc4random_uniform(4)`. `draw_asteroids` indexes the table instead of an if-chain. Covered: ragged-array problem, decay-forgets-length (one pointer spans all 4), FAM pitfall, value- vs pointer-constness, warnings≠errors. See NOTES.

## Known TODO / loose ends
- `apply_friction` takes `dt` but **doesn't use it** — drag is still a raw per-frame `× 0.99`, so framerate-dependent. Fix: `friction^dt` (`powf`) or `e^(−k·dt)` (`expf`). Invisible at locked 60fps. See NOTES "Framerate-independent damping".
- Debug `printf` of speed/angle still in the thrust path — remove once tuning's done.
- Ship is still a circle + nose line; eventual real **triangle** ship is a "make it pretty" item.

## Next up (roadmap)
1. **Bullets** — fire from ship nose along heading (`get_direction`), own position/velocity, lifetime/range limit, fixed-size pool with `alive` flag (the entity pattern from NOTES "Arrays").
2. **Collisions** — bounding-circle proxy + squared-distance test, cross-type only (asteroid×bullet, asteroid×ship). All the design groundwork is in NOTES "Collision detection".
3. **Asteroid destruction / splitting** — bullet hit removes/splits an asteroid into smaller ones.
4. **Ship death** — asteroid×ship contact blows up the ship.
5. Core gameplay done → **make it pretty** (triangle ship, vector asteroids, explosions, score).

## Pivot
Console rendering can't get close enough to original Asteroids (no smooth rotation, coarse grid, no real graphics). Switching to **raylib** for rendering: still C, single library, simple API, gives real 2D pixels, rotation, color, audio. The C-language learning goals on the "Next up" list are unchanged — only the rendering layer swaps. ncurses experiment stays in the repo as a reference for the linking/Makefile/game-loop shape.

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
