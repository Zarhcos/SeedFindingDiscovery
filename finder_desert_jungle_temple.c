// finder_desert_jungle_temple.c
//
// Searches for seeds where a Desert Pyramid (desert temple) and a Jungle
// Pyramid (jungle temple) generate within 500 blocks of each other. Those two
// structures need very different biomes, so having them close together is a
// nice uncommon find.
//
// Build:
//   gcc finder_desert_jungle_temple.c -o finder_desert_jungle_temple -I./cubiomes -L./cubiomes -lcubiomes -lm

#include "generator.h"
#include "finders.h"

#include <stdio.h>
#include <stdint.h>

#define MAX_POS 64

static long dist2(int x1, int z1, int x2, int z2)
{
    long dx = x1 - x2;
    long dz = z1 - z2;
    return dx * dx + dz * dz;
}

static int collect(Generator *g, int type, uint64_t seed,
                   int rmin, int rmax, Pos *out, int maxout)
{
    int n = 0;
    for (int rx = rmin; rx <= rmax; rx++)
    {
        for (int rz = rmin; rz <= rmax; rz++)
        {
            Pos p;
            if (!getStructurePos(type, MC_1_20, seed, rx, rz, &p))
                continue;
            if (!isViableStructurePos(type, g, p.x, p.z, 0))
                continue;
            if (n < maxout)
                out[n++] = p;
        }
    }
    return n;
}

int main(void)
{
    const uint64_t SEED_START = 0;
    const uint64_t SEED_COUNT = 1000000000ULL; // 1 billion
    const int      MAX_DIST   = 500;
    const long     MAX_DIST2  = (long)MAX_DIST * MAX_DIST;

    Generator g;
    setupGenerator(&g, MC_1_20, 0);

    Pos deserts[MAX_POS];
    Pos jungles[MAX_POS];

    for (uint64_t s = SEED_START; s < SEED_START + SEED_COUNT; s++)
    {
        uint64_t seed = s;
        applySeed(&g, DIM_OVERWORLD, seed);

        // Both pyramids share the same region grid, so scanning the same band
        // of regions for each and comparing pairs covers everything nearby.
        int nd = collect(&g, Desert_Pyramid, seed, -2, 2, deserts, MAX_POS);
        if (nd == 0)
            continue;

        int nj = collect(&g, Jungle_Pyramid, seed, -2, 2, jungles, MAX_POS);
        if (nj == 0)
            continue;

        int found = 0;
        Pos fd = {0, 0}, fj = {0, 0};

        for (int i = 0; i < nd && !found; i++)
        {
            for (int j = 0; j < nj && !found; j++)
            {
                if (dist2(deserts[i].x, deserts[i].z,
                          jungles[j].x, jungles[j].z) <= MAX_DIST2)
                {
                    found = 1;
                    fd = deserts[i];
                    fj = jungles[j];
                }
            }
        }

        if (found)
        {
            printf("seed %20llu : desert temple %d,%d  jungle temple %d,%d\n",
                   (unsigned long long)seed, fd.x, fd.z, fj.x, fj.z);
            fflush(stdout);
        }

        if ((s % 100000ULL) == 0)
            fprintf(stderr, "\rscanned %llu seeds...", (unsigned long long)s);
    }

    fprintf(stderr, "\ndone.\n");
    return 0;
}
