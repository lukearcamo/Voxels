#ifndef VOXELS_CHUNKUPDATER_H
#define VOXELS_CHUNKUPDATER_H

#include "dependencies/igsi/core/vec3.h"

#include <deque>
#include <mutex>

namespace Voxels {
    class Chunk;
    class ChunkGenerator;
    class ChunkManager;

    template <typename T>
    class SafeUniqueQueue {
    private:
        std::mutex m;
        std::deque<T> q;
    public:
        void push(T elem);
        bool pop(T &elem);
    };

    class ChunkUpdater {
    public:
        ChunkManager* chunkManager;
        ChunkGenerator* chunkGenerator;

        std::deque<float> fillQueue;
        SafeUniqueQueue<float> populateQueue;
        SafeUniqueQueue<float> buildQueue;
        SafeUniqueQueue<float> mapQueue;

        ChunkUpdater(ChunkManager* chunkManager, ChunkGenerator* chunkGenerator);

        void updateGeometry(Chunk &chunk);
        void rebuildNeighborChunks(Igsi::vec3 coords, Igsi::vec3 local);
        
        void fillNext();
        void populateNext();
        void buildNext();
        void mapNextAll();
    };
}

#endif