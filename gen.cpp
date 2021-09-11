#include "gen.h"
#include "chunk.h"
#include "chunkManager.h"

#include "dependencies/igsi/core/vec3.h"

#include <cmath>

using namespace Igsi;

namespace Voxels {
    float mix(float x, float y, float a) { return x * (1.0 - a) + y * a; }
    float clamp(float x, float min, float max) { return std::fmin(std::fmax(x, min), max); }
    float smoothstep(float a, float b, float x) { return clamp(x * x * (3.0 - 2.0 * x), 0.0, 1.0); }

    // unsigned int murmur(float x, unsigned int seed) {
    //     // https://stackoverflow.com/questions/12342926/casting-float-to-int-bitwise-in-c
    //     unsigned int k = *(unsigned int*)&x;
    //     unsigned int h = seed;

    //     k *= 0xcc9e2d51;
    //     k ^= k << 15;
    //     k *= 0x1b873593;

    //     h ^= k;
    //     h = h << 13;
    //     h = h * 5 + 0xe6546b64;
        
    //     // Finalizer
    //     h ^= h >> 16;
    //     h *= 0x85ebca6b;
    //     h ^= h >> 13;
    //     h *= 0xc2b2ae35;
    //     h ^= h >> 16;
    //     return h;
    // }
    // float murmur3(vec3 p) {
    //     unsigned int seed = 0x12345678;
    //     seed = murmur(p.x, seed);
    //     seed = murmur(p.y, seed);
    //     seed = murmur(p.z, seed);
    //     return seed / 0xffffffff;
    // }



    // I dont think this functions is completely even distribution either
    // Since it doesnt need to be normalized, we dont have to worry about samples bunching up at the corners

    // By Inigo Quilez, from https://www.shadertoy.com/view/Xsl3Dl
    vec3 hash(vec3 p) {
        p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
                 dot(p, vec3(269.5, 183.3, 246.1)),
                 dot(p, vec3(113.5, 271.9, 124.6)));
        p = vec3(std::sin(p.x), std::sin(p.y), std::sin(p.z));
        p *= 43758.5453123;
        return (p - floor(p)) * 2.0 - 1.0;
    }
    float perlin3d(vec3 p) {
        vec3 i = floor(p);
        vec3 f = p - i;
        vec3 u = f * f * (f * -2.0 + 3.0);
        return mix(mix(mix(dot(hash(i + vec3(0.0, 0.0, 0.0)), f - vec3(0.0, 0.0, 0.0)), 
                           dot(hash(i + vec3(1.0, 0.0, 0.0)), f - vec3(1.0, 0.0, 0.0)), u.x),
                       mix(dot(hash(i + vec3(0.0, 1.0, 0.0)), f - vec3(0.0, 1.0, 0.0)), 
                           dot(hash(i + vec3(1.0, 1.0, 0.0)), f - vec3(1.0, 1.0, 0.0)), u.x), u.y),
                   mix(mix(dot(hash(i + vec3(0.0, 0.0, 1.0)), f - vec3(0.0, 0.0, 1.0)), 
                           dot(hash(i + vec3(1.0, 0.0, 1.0)), f - vec3(1.0, 0.0, 1.0)), u.x),
                       mix(dot(hash(i + vec3(0.0, 1.0, 1.0)), f - vec3(0.0, 1.0, 1.0)), 
                           dot(hash(i + vec3(1.0, 1.0, 1.0)), f - vec3(1.0, 1.0, 1.0)), u.x), u.y), u.z);
    }
    float yGradient(float y) {
        // return std::fmin(std::fmax(y / -16.0, 0.0), 1.0);
        // return clamp(y / -16.0, 0.0, 1.0);
        // return clamp(y / -16.0 - 1.0, -1.0, 0.0);
        // return clamp(y / -16.0 - 1.0, -0.5, 0.0);
        return clamp(y / -8.0 - 1.0, -1.0, 0.0);
    }

    void ChunkGenerator::fillTerrain(Chunk* chunk) {
        for (int z = 0; z < chunkDims.z; z++) {
            for (int y = 0; y < chunkDims.y; y++) {
                for (int x = 0; x < chunkDims.x; x++) {
                    // chunk->setVoxel(vec3(x, y, z), 2);

                    // Worst case -- 5x5x5 causes 100% GPU:
                    // bool state = (x + y + z) % 2 == 0;
                    // chunk->setVoxel(vec3(x, y, z), state ? 2 : 0);

                    // vec3 state = hash(vec3(x, y, z));
                    // chunk->setVoxel(vec3(x, y, z), (state.x + state.y + state.z) / 3.0 > -0.9 ? 2 : 0);

                    vec3 local = vec3(x, y, z);
                    float state = perlin3d(local / chunkDims + chunk->coords);
                    state += yGradient(y + chunk->coords.y * chunkDims.y);
                    chunk->setVoxel(local, state > 0.0 ? 2 : 0);
                }
            }
        }
    }
    void ChunkGenerator::populateTerrain(Chunk* chunk, ChunkManager* chunkManager) {
        for (int z = 0; z < chunkDims.z; z++) {
            for (int y = 0; y < chunkDims.y; y++) {
                for (int x = 0; x < chunkDims.x; x++) {
                    vec3 local = vec3(x, y, z);
                    char blockType = chunk->getVoxel(local);

                    // CANNOT ACCESS GLOBAL VOXEL WITHOUT CHUNKMANAGER -- THIS MUST STAY
                    // CHUNKMANAGER NEEDS THIS FUNCTION -- SO THIS MUST CHANGE
                    // CIRCULAR DEPENDENCY
                    char above = chunkManager->getVoxelGlobal(local + vec3(0.0, 1.0, 0.0) + chunk->coords * chunkDims);

                    if (blockType != 0) {
                        if (above == 0) {
                            chunk->setVoxel(local, 1);
                        }
                    }
                }
            }
        }
    }
}