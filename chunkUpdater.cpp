#include "chunkUpdater.h"
#include "chunkManager.h"
#include "chunk.h"
#include "gen.h"

#include <glad/gl.h>

#include "dependencies/igsi/core/vec3.h"
#include "dependencies/igsi/core/mat4.h"
#include "dependencies/igsi/core/helpers.h"
#include "dependencies/igsi/core/transform.h"

#include <iostream>
#include <thread>
#include <cmath>
#include <map>
#include <algorithm>

using namespace Igsi;

namespace Voxels {
    template <typename T>
    void SafeUniqueQueue<T>::push(T elem) {
        std::lock_guard<std::mutex> lock(m);
        // Without checks -- Max 19 elements at a time
        // With find -- Max 4
        // With back -- Around 5, Max 11
        if (std::find(q.begin(), q.end(), elem) == q.end()) {
        // if (q.empty() || q.back() != elem) {
            q.push_back(elem);
        }
    }
    template <typename T>
    bool SafeUniqueQueue<T>::pop(T &elem) {
        if (q.empty()) return false;
        std::lock_guard<std::mutex> lock(m);
        elem = q.front();
        q.pop_front();
        return true;
    }


    ChunkUpdater::ChunkUpdater(ChunkManager* chunkManager, ChunkGenerator* chunkGenerator) {
        this->chunkManager = chunkManager;
        this->chunkGenerator = chunkGenerator;
    }

    // Why is this here instead of inside Chunk? Because it requires access to global chunk data
    void ChunkUpdater::updateGeometry(Chunk &chunk) {
        chunk.geometryData.clear();

        int tmpDrawCount = 0;
        // We still need tmpDrawCount because while we are still in the middle of incrementing chunk.drawCount,
        // the existing chunk geometry is being drawn with incomplete draw count
        // So we have to update it once at the end

        char N[3][3][3];

        for (int i = 0; i < chunk.data.size(); i++) {
            char currentBlock = chunk.data.at(i);
            if (currentBlock == 0) continue;

            vec3 local = vec3(
                std::fmod(i, chunkDims.x),
                std::fmod(std::floor(i / chunkDims.x), chunkDims.y),
                std::floor(i / (chunkDims.x * chunkDims.y))
            );
            vec3 voxel = chunk.coords * chunkDims + local;

            vec3 q = local - 1.0;
            for (int nz = 0; nz < 3; nz++) {
                for (int ny = 0; ny < 3; ny++) {
                    for (int nx = 0; nx < 3; nx++) {
                        vec3 r = q + vec3(nx, ny, nz);
                        if (r.x < 0 || r.y < 0 || r.z < 0 || r.x > 15 || r.y > 15 || r.z > 15) {
                            vec3 c = ChunkManager::getChunkCoords(r) + chunk.coords;
                            vec3 l = ChunkManager::getLocalCoords(r);
                            float id = ChunkManager::coordsToId(c);
                            N[nx][ny][nz] = chunkManager->hasChunk(id) ? chunkManager->getChunk(id).getVoxel(l) : 0;
                            // if (hasChunk(id)) {
                            //     N[nx][ny][nz] = getChunk(id).getVoxel(l);
                            // }
                            // else {
                            //     N[nx][ny][nz] = (
                            //         (r.x < 0 || r.z < 0 || r.x > 15 || r.z > 15)
                            //         && r.y >= 0
                            //         && r.y <= 15
                            //     ) ? 1 : 0;
                            // }
                        }
                        else {
                            N[nx][ny][nz] = chunk.getVoxel(r);
                        }
                    }
                }
            }

            if (!N[1][1][2]) tmpDrawCount += chunk.addCubeFace(0, currentBlock, local, N); // N
            if (!N[1][1][0]) tmpDrawCount += chunk.addCubeFace(1, currentBlock, local, N); // S
            if (!N[2][1][1]) tmpDrawCount += chunk.addCubeFace(2, currentBlock, local, N); // E
            if (!N[0][1][1]) tmpDrawCount += chunk.addCubeFace(3, currentBlock, local, N); // W
            if (!N[1][2][1]) tmpDrawCount += chunk.addCubeFace(4, currentBlock, local, N); // T
            if (!N[1][0][1]) tmpDrawCount += chunk.addCubeFace(5, currentBlock, local, N); // B
        }
        chunk.drawCount = tmpDrawCount;
    }

    void ChunkUpdater::rebuildNeighborChunks(vec3 coords, vec3 local) {
        vec3 upperBound = chunkDims; // Copying to another variable removes chunkDim's "constness"
        upperBound -= 1.0;
        float x = std::trunc(local.x / upperBound.x * 2.0 - 1);
        float y = std::trunc(local.y / upperBound.y * 2.0 - 1);
        float z = std::trunc(local.z / upperBound.z * 2.0 - 1);

        if (x != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(x, 0, 0));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }
        if (y != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(0, y, 0));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }
        if (z != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(0, 0, z));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }

        // Although these neighbors are not necessarily touching the chunk, their AO is still affected

        if (x != 0 && y != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(x, y, 0));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }
        if (x != 0 && z != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(x, 0, z));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }
        if (y != 0 && z != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(0, y, z));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }

        if (x != 0 && y != 0 && z != 0) {
            float id = ChunkManager::coordsToId(coords + vec3(x, y, z));
            if (chunkManager->hasChunk(id)) buildQueue.push(id);
        }
    }

    void ChunkUpdater::fillNext() {
        if (!fillQueue.empty()) {
            float nextId = fillQueue.front();
            fillQueue.pop_front();
            Chunk &chunk = chunkManager->getChunk(nextId);

            chunkGenerator->fillTerrain(&chunk);
            
            for (int nz = -1; nz <= 1; nz++) {
                for (int ny = -1; ny <= 1; ny++) {
                    for (int nx = -1; nx <= 1; nx++) {
                        if (nx == 0 && ny == 0 && nz == 0) continue;
                        float id = ChunkManager::coordsToId(chunk.coords + vec3(nx, ny, nz));
                        if (chunkManager->hasChunk(id)) {
                            chunk.numNeighbors += 1;
                            chunkManager->getChunk(id).numFilledNeighbors += 1;
                        }
                    }
                }
            }
            populateQueue.push(nextId);
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    void ChunkUpdater::populateNext() {
        float nextId;
        if (populateQueue.pop(nextId)) {
            Chunk &chunk = chunkManager->getChunk(nextId);

            if (chunk.numFilledNeighbors == chunk.numNeighbors) {
                chunkGenerator->populateTerrain(&chunk, chunkManager);

                for (int nz = -1; nz <= 1; nz++) {
                    for (int ny = -1; ny <= 1; ny++) {
                        for (int nx = -1; nx <= 1; nx++) {
                            if (nx == 0 && ny == 0 && nz == 0) continue;
                            float id = ChunkManager::coordsToId(chunk.coords + vec3(nx, ny, nz));
                            if (chunkManager->hasChunk(id)) {
                                chunkManager->getChunk(id).numPopulatedNeighbors += 1;
                            }
                        }
                    }
                }
                buildQueue.push(nextId);
            }
            else {
                populateQueue.push(nextId);
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void ChunkUpdater::buildNext() {
        float nextId;
        if (buildQueue.pop(nextId)) {
            Chunk &chunk = chunkManager->getChunk(nextId);

            if (chunk.numPopulatedNeighbors == chunk.numNeighbors) {
                updateGeometry(chunk);
                mapQueue.push(nextId);
            }
            else {
                buildQueue.push(nextId);
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    void ChunkUpdater::mapNextAll() {
        float nextId;
        while (mapQueue.pop(nextId)) {
            Chunk &chunk = chunkManager->getChunk(nextId);
            // int numComponents = chunk.drawCount * Chunk::STRIDE;

            glBindBuffer(GL_ARRAY_BUFFER, chunk.VBO);
            void* bufferPtr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
            memcpy(bufferPtr, chunk.geometryData.data(), chunk.geometryData.size() * sizeof(GLuint)); // cannot sizeof(vector) because sizeof is compile time but vector is runtime
            glUnmapBuffer(GL_ARRAY_BUFFER);

            // Do not do this because we already clear() at start of updategeo,
            // it is inefficient to reserve all the time,
            // and also it can cause crashes if you reserve all the time
            // chunk.geometryData.clear();
            // chunk.geometryData.shrink_to_fit();
        }
    }
}