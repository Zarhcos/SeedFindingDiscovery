// finder_monument_spawn.c
//
// Searches for seeds with an Ocean Monument within 100 blocks of world spawn.
// Spawning almost on top of a guardian farm's worth of prismarine is a fun,
// uncommon property.
//
// Build:
//   gcc finder_monument_spawn.c -o finder_monument_spawn -I./cubiomes -L./cubiomes -lcubiomes -lm

#include "generator.h"
#include "finders.h"

#include <stdio.h>
#include <stdint.h>

// Squared distance between two points (avoids a sqrt).
static long dist2(int x1, int z1, int x2, int z2)
{
    long dx = x1 - x2;
    long dz = z1 - z2;
    return dx * dx + dz * dz;
}

int main(void)
{
    const uint64_t SEED_START = 0;
    const uint64_t SEED_COUNT = 1000000000ULL; // 1 billion
    const int      RADIUS     = 100;           // blocks from spawn
    const long     RADIUS2    = (long)RADIUS * RADIUS;

    Generator g;
    setupGenerator(&g, MC_1_20, 0);

    for (uint64_t s = SEED_START; s < SEED_START + SEED_COUNT; s++)
    {
        uint64_t seed = s;
        applySeed(&g, DIM_OVERWORLD, seed);

        Pos spawn = getSpawn(&g);

        // Monuments are placed on a region grid. A 100-block radius around
        // spawn can only touch the handful of regions immediately around it,
        // so we figure out which region spawn falls in and scan a 3x3 block of
        // regions centered on it.
        StructureConfig sc;
        getStructureConfig(Monument, MC_1_20, &sc);
        int regBlocks = sc.regionSize * 16; // region size in blocks

        int rsx = spawn.x / regBlocks;
        int rsz = spawn.z / regBlocks;

        int found = 0;
        Pos mp = {0, 0};

        for (int rx = rsx - 1; rx <= rsx + 1 && !found; rx++)
        {
            for (int rz = rsz - 1; rz <= rsz + 1 && !found; rz++)
            {
                Pos p;
                // getStructurePos returns 1 if the region *could* contain the
                // structure and fills p with its candidate position.
                if (!getStructurePos(Monument, MC_1_20, seed, rx, rz, &p))
                    continue;

                if (dist2(spawn.x, spawn.z, p.x, p.z) > RADIUS2)
                    continue;

                // isViableStructurePos confirms the biomes there actually allow
                // a monument to generate (it's a candidate until verified).
                if (!isViableStructurePos(Monument, &g, p.x, p.z, 0))
                    continue;

                found = 1;
                mp = p;
            }
        }

        if (found)
        {
            printf("seed %20llu : ocean monument at %d,%d (spawn %d,%d)\n",
                   (unsigned long long)seed, mp.x, mp.z, spawn.x, spawn.z);
            fflush(stdout);
        }

        if ((s % 100000ULL) == 0)
            fprintf(stderr, "\rscanned %llu seeds...", (unsigned long long)s);
    }

    fprintf(stderr, "\ndone.\n");
    return 0;
}
