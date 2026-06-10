# Minecraft Seed Finders (Java Edition 1.20)

A small collection of standalone C programs that use the
[cubiomes](https://github.com/Cubitect/cubiomes) library to hunt for Minecraft
seeds with uncommon world-generation properties — no need to launch the game.

Each finder is a self-contained program that scans a large range of seeds and
prints any match (with coordinates) to standard output.

## Project structure

```
SF/
├── cubiomes/                       # the cubiomes library (cloned & compiled)
│   └── libcubiomes.a               # static lib + headers (generator.h, finders.h)
├── finder_mushroom_spawn.c         # mushroom island within 500 blocks of spawn
├── finder_swamp_spawn.c            # world spawn lands inside a swamp
├── finder_monument_spawn.c         # ocean monument within 100 blocks of spawn
├── finder_village_mansion.c        # village + woodland mansion within 300 blocks
├── finder_desert_jungle_temple.c   # desert temple + jungle temple within 500 blocks
└── README.md
```

Every `.c` file is independent — there is no shared header or build system to
learn. Each one targets **Minecraft Java Edition 1.20** (`MC_1_20`).

## Compiling

Each finder compiles with the same command (swap in the file name):

```sh
gcc finder_mushroom_spawn.c      -o finder_mushroom_spawn      -I./cubiomes -L./cubiomes -lcubiomes -lm
gcc finder_swamp_spawn.c         -o finder_swamp_spawn         -I./cubiomes -L./cubiomes -lcubiomes -lm
gcc finder_monument_spawn.c      -o finder_monument_spawn      -I./cubiomes -L./cubiomes -lcubiomes -lm
gcc finder_village_mansion.c     -o finder_village_mansion     -I./cubiomes -L./cubiomes -lcubiomes -lm
gcc finder_desert_jungle_temple.c -o finder_desert_jungle_temple -I./cubiomes -L./cubiomes -lcubiomes -lm
```

- `-I./cubiomes` tells gcc where the headers are.
- `-L./cubiomes -lcubiomes` links the compiled static library.
- `-lm` links the math library (cubiomes needs it).

## Running

Just run the compiled binary:

```sh
./finder_mushroom_spawn
```

- **Matches** are printed to **stdout** (one line per seed).
- A **progress counter** is printed to **stderr** so it doesn't pollute results.

That means you can save only the matches to a file while still watching progress:

```sh
./finder_village_mansion > hits.txt
```

Each finder scans seeds `0 .. 1,000,000,000`. You can change `SEED_START` and
`SEED_COUNT` near the top of any `main()` to scan a different range (e.g. to
split work across terminals).

### A note on speed

The two "near spawn" finders (`mushroom`, `swamp`, `monument`) call
`getSpawn()`, which computes the real world spawn point and is relatively
expensive — expect these to be slower. The structure-pair finders only do
cheap region math plus biome checks, so they rip through seeds much faster.

## How to write your own custom criterion

The whole library boils down to a handful of calls. Here is the skeleton every
finder uses:

```c
#include "generator.h"
#include "finders.h"
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    Generator g;
    setupGenerator(&g, MC_1_20, 0);          // set version ONCE

    for (uint64_t seed = 0; seed < 1000000000ULL; seed++)
    {
        applySeed(&g, DIM_OVERWORLD, seed);  // load this seed

        /* ---- your test goes here ---- */

        // if (matches) printf("seed %llu : ...\n", (unsigned long long)seed);
    }
    return 0;
}
```

The two building blocks you combine inside the loop:

### 1. Checking the biome at a location

```c
// scale = 4 -> coordinates are block >> 2 (biome resolution).
// y = 15 is roughly surface height (block y ~ 60) in 1.18+.
int biome = getBiomeAt(&g, 4, blockX >> 2, 15, blockZ >> 2);
if (biome == mushroom_fields) { /* ... */ }
```

Biome names (`swamp`, `jungle`, `desert`, `mushroom_fields`, ...) are constants
defined in `cubiomes/biomes.h`.

### 2. Finding a structure

```c
StructureConfig sc;
getStructureConfig(Village, MC_1_20, &sc);   // sc.regionSize is in chunks
int regBlocks = sc.regionSize * 16;

Pos p;
// getStructurePos: does region (rx, rz) place this structure? fills p.
if (getStructurePos(Village, MC_1_20, seed, rx, rz, &p))
{
    // isViableStructurePos: do the biomes there actually allow it?
    if (isViableStructurePos(Village, &g, p.x, p.z, 0))
    {
        // p.x, p.z is a real structure location.
    }
}
```

Structures are placed on a region grid. To search an area, loop over the region
coordinates `(rx, rz)` that overlap it. Structure name constants (`Village`,
`Mansion`, `Monument`, `Desert_Pyramid`, `Jungle_Pyramid`, `Swamp_Hut`,
`Outpost`, `Ruined_Portal`, ...) live in `cubiomes/finders.h`.

### 3. The world spawn point

```c
Pos spawn = getSpawn(&g);    // expensive; call at most once per seed
```

### Putting it together — recipe for a new finder

1. Copy the closest existing finder to `finder_myidea.c`.
2. Decide what you're testing:
   - *biome property* → use `getBiomeAt` over a grid of points.
   - *structure property* → use `getStructurePos` + `isViableStructurePos`,
     looping over region coordinates.
   - *relationship between two things* → collect positions of each into arrays,
     then compare them (see `finder_village_mansion.c` for the distance pattern).
3. Print a match line to stdout when your condition holds.
4. Compile with the `gcc` command above and run it.

Tip: start with a *loose* condition and tighten it. If nothing prints in a few
minutes, your criterion is probably rarer than you think — loosen the distance
or radius and confirm matches appear, then make it stricter.
