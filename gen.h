#ifndef VOXELS_GEN_H
#define VOXELS_GEN_H

#include <glad/gl.h>

#include "dependencies/igsi/core/vec3.h"

namespace Voxels {
    class Chunk;
    class ChunkManager;

    // MAKE THESE ALL MEMBERS OF CHUNKGENERATOR
    // Or part of a math/utils file
    float mix(float x, float y, float a);
    float clamp(float x, float min, float max);
    float smoothstep(float a, float b, float x);

    Igsi::vec3 hash(Igsi::vec3 p);
    float perlin3d(Igsi::vec3 p);
    float yGradient(float y);

    class ChunkGenerator {
    public:
        void fillTerrain(Chunk* chunk);
        void populateTerrain(Chunk* chunk, ChunkManager* chunkManager);
    };
}

#endif