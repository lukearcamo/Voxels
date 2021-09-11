#ifndef VOXELS_CHUNKMANAGER_H
#define VOXELS_CHUNKMANAGER_H

#include <glad/gl.h>

#include "dependencies/igsi/core/vec3.h"
#include "dependencies/igsi/core/mat4.h"
#include "dependencies/igsi/core/transform.h"

#include <map>
#include <deque>
#include <mutex>

namespace Voxels {
    class Chunk;
    class ChunkGenerator;
    class Frustum;

    class ChunkManager {
    public:
        static float coordsToId(Igsi::vec3 coords);
        static Igsi::vec3 getChunkCoords(Igsi::vec3 voxel);
        static Igsi::vec3 getLocalCoords(Igsi::vec3 voxel);
        
        std::map<float, Chunk> chunks;

        Chunk &addChunk(Igsi::vec3 coords); // Note how this takes a coordinate, not an ID -- MB we should change to ID for consistency?
        bool hasChunk(float id);
        Chunk &getChunk(float id);
        void deleteChunk(float id);

        char getVoxelGlobal(Igsi::vec3 voxel);
        void setVoxelGlobal(Igsi::vec3 voxel, char blockType); // If you try to set voxel in nonexistent chunk, it will create new chunk

        void raycastVoxels(Igsi::vec3 ro, Igsi::vec3 rd, float distance, Igsi::vec3 &voxel, Igsi::vec3 &normal);
        void drawChunks(Igsi::Transform* camera, Igsi::mat4 projectionMatrix, Frustum* frustum);
    };
}

#endif