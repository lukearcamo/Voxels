#include "chunkManager.h"
#include "chunk.h"
#include "gen.h"
#include "frustum.h"

#include <glad/gl.h>

#include "dependencies/igsi/core/vec3.h"
// The dependencies below here are all basically just for drawChunks. Maybe we can somehow eliminate them, move drawChunks somewhere else?
#include "dependencies/igsi/core/vec4.h"
#include "dependencies/igsi/core/mat4.h"
#include "dependencies/igsi/core/helpers.h"
#include "dependencies/igsi/core/transform.h"

#include <cmath>
#include <map>
#include <algorithm>

#include <iostream>

using namespace Igsi;

namespace Voxels {
    float ChunkManager::coordsToId(vec3 coords) {
        coords.x = std::fabs(coords.x * 2.0) - (coords.x < 0.0); // Fold function (maps negative & positive integers -> positive integers)
        coords.y = std::fabs(coords.y * 2.0) - (coords.y < 0.0);
        coords.z = std::fabs(coords.z * 2.0) - (coords.z < 0.0);
        // cantorPair(cantorPair(a, b), c) for unique integer for every trio of positive integers
        float sum = coords.x + coords.y;
        float cantor1 = 0.5 * sum * (sum + 1) + coords.y;
        sum = cantor1 + coords.z;
        return 0.5 * sum * (sum + 1) + coords.z;
    }
    vec3 ChunkManager::getChunkCoords(vec3 voxel) { return floor(voxel / chunkDims); }
    vec3 ChunkManager::getLocalCoords(vec3 voxel) {
        return vec3(
            std::fmod((std::fmod(voxel.x, chunkDims.x) + chunkDims.x), chunkDims.x),
            std::fmod((std::fmod(voxel.y, chunkDims.y) + chunkDims.y), chunkDims.y),
            std::fmod((std::fmod(voxel.z, chunkDims.z) + chunkDims.z), chunkDims.z)
        );
    }

    Chunk& ChunkManager::addChunk(vec3 coords) {
        return chunks.emplace(coordsToId(coords), Chunk(coords)).first->second;
    }
    bool ChunkManager::hasChunk(float id) {
        return chunks.find(id) != chunks.end();
    }
    Chunk& ChunkManager::getChunk(float id) {
        return chunks.at(id);
    }
    void ChunkManager::deleteChunk(float id) {
        // std::vector<char>().swap(chunks.at(id).data);
        chunks.at(id).data.clear();
        chunks.at(id).data.shrink_to_fit();
        chunks.erase(id);
    }

    char ChunkManager::getVoxelGlobal(vec3 voxel) {
        float id = coordsToId(getChunkCoords(voxel));
        vec3 local = getLocalCoords(voxel);
        return hasChunk(id) ? getChunk(id).getVoxel(local) : 0;
    }
    void ChunkManager::setVoxelGlobal(vec3 voxel, char blockType) {
        vec3 coords = getChunkCoords(voxel);
        vec3 local = getLocalCoords(voxel);
        addChunk(coords).setVoxel(local, blockType); // If you try to set voxel in nonexistent chunk, it will create new chunk
    }

    // Find more efficient way to raycast
    void ChunkManager::raycastVoxels(vec3 ro, vec3 rd, float distance, vec3 &voxel, vec3 &normal) {
        vec3 lastVoxel;
        for (float t = 0.0; t < distance; t += 0.01) {
            vec3 p = ro + rd * t;

            voxel = floor(p);
            if (voxel == lastVoxel) continue;
            lastVoxel = voxel;

            vec3 coords = getChunkCoords(voxel);
            vec3 local = getLocalCoords(voxel);

            p -= voxel + 0.5;

            normal = vec3(0.0);
            vec3 q = abs(p);

            float maximum = std::fmax(q.x, std::fmax(q.y, q.z));

            // Sign function
            if (q.x == maximum) normal.x = (p.x > 0) - (p.x < 0);
            if (q.y == maximum) normal.y = (p.y > 0) - (p.y < 0);
            if (q.z == maximum) normal.z = (p.z > 0) - (p.z < 0);

            if (getVoxelGlobal(voxel)) return;
        }
        normal = vec3(0.0);
        return;
    }
    void ChunkManager::drawChunks(Transform* camera, mat4 projectionMatrix, Frustum* frustum) {
        mat4 worldMatrix;

        for (auto it = chunks.begin(); it != chunks.end(); ++it) {
            float radius = length(vec3(0.5) * chunkDims); // Should be constant
            vec4 center = vec4(
                chunkDims.x * (0.5 + it->second.coords.x),
                chunkDims.y * (0.5 + it->second.coords.y),
                chunkDims.z * (0.5 + it->second.coords.z),
                1.0
            );
            center = camera->inverseWorldMatrix * center;

            if (frustum->intersectsSphere(vec3(center.x, center.y, center.z), radius)) {
                glBindVertexArray(it->second.VAO);
                GLuint current = getCurrentShaderProgram();

                worldMatrix.setTranslation(it->second.coords * chunkDims);
                setUniform("worldMatrix", worldMatrix, current);

                setUniform("viewMatrix", camera->inverseWorldMatrix, current);
                setUniform("projectionMatrix", projectionMatrix, current);

                setUniform("cameraPosition", camera->position, current);
                glDrawArrays(GL_TRIANGLES, 0, it->second.drawCount);
            }
        }
    }
}