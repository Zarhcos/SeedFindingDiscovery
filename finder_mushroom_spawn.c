// finder_mushroom_spawn.c
//
// Searches for seeds where a Mushroom Fields biome (the "mushroom island")
// appears within 500 blocks of the world spawn point.
//
// Mushroom Fields is one of the rarest overworld biomes, so having one sitting
// right next to where you spawn is an uncommon-but-not-insane property.
//
// Build:
//   gcc finder_mushroom_spawn.c -o finder_mushroom_spawn -I./cubiomes -L./cubiomes -lcubiomes -lm

#include "generator.h"
#include "finders.h"

#include <stdio.h>
#include <stdint.h>

int main(void)
{
    // How many seeds to scan, and how far from spawn we look.
    const uint64_t SEED_START = 0;
    const uint64_t SEED_COUNT = 1000000000ULL; // 1 billion
    const int      RADIUS     = 500;           // blocks around spawn
    const int      STEP       = 32;            // sample spacing (blocks)

    // A Generator holds all the world-generation state. We set it up ONCE for
    // the target Minecraft version and then re-apply a new seed each loop.
    Generator g;
    setupGenerator(&g, MC_1_20, 0);

    for (uint64_t s = SEED_START; s < SEED_START + SEED_COUNT; s++)
    {
        uint64_t seed = s;

        // applySeed loads this specific seed into the generator (overworld).
        applySeed(&g, DIM_OVERWORLD, seed);

        // getSpawn computes the actual world spawn point. This is a relatively
        // expensive call, which is why this finder is slower than the
        // structure-only finders.
        Pos spawn = getSpawn(&g);

        int found = 0;
        int fx = 0, fz = 0;

        // Sample a grid of points around spawn looking for mushroom_fields.
        for (int dx = -RADIUS; dx <= RADIUS && !found; dx += STEP)
        {
            for (int dz = -RADIUS; dz <= RADIUS && !found; dz += STEP)
            {
                int bx = spawn.x + dx;
                int bz = spawn.z + dz;

                // getBiomeAt(g, scale, x, y, z): with scale = 4 the x/y/z are
                // in "biome coordinates" (block >> 2). y = 15 corresponds to a
                // surface height of about y=60. This is the standard way to ask
                // "what surface biome is at this block?" in 1.18+.
                int biome = getBiomeAt(&g, 4, bx >> 2, 15, bz >> 2);

                if (biome == mushroom_fields)
                {
                    found = 1;
                    fx = bx;
                    fz = bz;
                }
            }
        }

        if (found)
        {
            printf("seed %20llu : mushroom_fields near spawn "
                   "(spawn %d,%d  biome at %d,%d)\n",
                   (unsigned long long)seed, spawn.x, spawn.z, fx, fz);
            fflush(stdout);
        }

        // Light progress indicator on stderr so stdout stays clean for results.
        if ((s % 100000ULL) == 0)
            fprintf(stderr, "\rscanned %llu seeds...", (unsigned long long)s);
    }

    fprintf(stderr, "\ndone.\n");
    return 0;
}
