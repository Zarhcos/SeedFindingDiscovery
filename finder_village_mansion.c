// finder_village_mansion.c
//
// Searches for seeds where a Village and a Woodland Mansion generate within
// 300 blocks of each other. Mansions are rare and normally sit deep in dark
// forests, so finding a village hugging one is uncommon.
//
// Build:
//   gcc finder_village_mansion.c -o finder_village_mansion -I./cubiomes -L./cubiomes -lcubiomes -lm

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

// Collect every viable structure of `type` whose region coordinate is in
// [rmin, rmax] x [rmin, rmax]. Returns the count written into `out`.
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
            // Confirm the biomes actually support this structure.
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
    const int      MAX_DIST   = 300;
    const long     MAX_DIST2  = (long)MAX_DIST * MAX_DIST;

    Generator g;
    setupGenerator(&g, MC_1_20, 0);

    Pos mansions[MAX_POS];
    Pos villages[MAX_POS];

    for (uint64_t s = SEED_START; s < SEED_START + SEED_COUNT; s++)
    {
        uint64_t seed = s;
        applySeed(&g, DIM_OVERWORLD, seed);

        // Mansions are very spread out (large region size), so a small band of
        // regions around the origin already covers a wide area. We scan the
        // same region band for villages and then compare every pair.
        int nm = collect(&g, Mansion, seed, -1, 1, mansions, MAX_POS);
        if (nm == 0)
            continue; // no mansion here, nothing to pair with

        int nv = collect(&g, Village, seed, -2, 2, villages, MAX_POS);

        int found = 0;
        Pos fm = {0, 0}, fv = {0, 0};

        for (int i = 0; i < nm && !found; i++)
        {
            for (int j = 0; j < nv && !found; j++)
            {
                if (dist2(mansions[i].x, mansions[i].z,
                          villages[j].x, villages[j].z) <= MAX_DIST2)
                {
                    found = 1;
                    fm = mansions[i];
                    fv = villages[j];
                }
            }
        }

        if (found)
        {
            printf("seed %20llu : mansion %d,%d  village %d,%d\n",
                   (unsigned long long)seed, fm.x, fm.z, fv.x, fv.z);
            fflush(stdout);
        }

        if ((s % 100000ULL) == 0)
            fprintf(stderr, "\rscanned %llu seeds...", (unsigned long long)s);
    }

    fprintf(stderr, "\ndone.\n");
    return 0;
}
