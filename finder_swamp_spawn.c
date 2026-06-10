// finder_swamp_spawn.c
//
// Searches for seeds where the world spawn point itself lands inside a
// Swamp biome. Spawn is normally biased toward "nice" biomes, so landing
// directly in a swamp is uncommon.
//
// Build:
//   gcc finder_swamp_spawn.c -o finder_swamp_spawn -I./cubiomes -L./cubiomes -lcubiomes -lm

#include "generator.h"
#include "finders.h"

#include <stdio.h>
#include <stdint.h>

int main(void)
{
    const uint64_t SEED_START = 0;
    const uint64_t SEED_COUNT = 1000000000ULL; // 1 billion

    Generator g;
    setupGenerator(&g, MC_1_20, 0);

    for (uint64_t s = SEED_START; s < SEED_START + SEED_COUNT; s++)
    {
        uint64_t seed = s;
        applySeed(&g, DIM_OVERWORLD, seed);

        // Compute the real spawn point for this seed.
        Pos spawn = getSpawn(&g);

        // Ask for the surface biome exactly at the spawn block.
        int biome = getBiomeAt(&g, 4, spawn.x >> 2, 15, spawn.z >> 2);

        // mangrove_swamp is the warm variant; we accept either as "a swamp".
        if (biome == swamp || biome == mangrove_swamp)
        {
            printf("seed %20llu : spawn is in %s at %d,%d\n",
                   (unsigned long long)seed,
                   (biome == swamp) ? "swamp" : "mangrove_swamp",
                   spawn.x, spawn.z);
            fflush(stdout);
        }

        if ((s % 100000ULL) == 0)
            fprintf(stderr, "\rscanned %llu seeds...", (unsigned long long)s);
    }

    fprintf(stderr, "\ndone.\n");
    return 0;
}
