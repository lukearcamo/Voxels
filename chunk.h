#ifndef VOXELS_CHUNK_H
#define VOXELS_CHUNK_H

#include <glad/gl.h>

#include "dependencies/igsi/core/vec3.h"
#include "dependencies/igsi/core/mat4.h"
#include "dependencies/igsi/core/helpers.h"

#include <vector>
#include <map>
#include <deque>
#include <mutex>

namespace Voxels {
    // Technically should be integer vector but whatever
    extern const Igsi::vec3 chunkDims;

    extern const int NUM_VOXELS;
    extern const int MAX_VERTS;

    extern const float faceVertexData[6][18];
    extern const char uvData[12];
    
    // https://stackoverflow.com/questions/51939692/c-extern-constant-int-for-array-size
    extern const int TOTAL_NUM_BLOCK_TYPES;
    extern const char atlasLUT[][6]; //[TOTAL_NUM_BLOCK_TYPES + 1]
    
    class Chunk {
    private:
        // https://stackoverflow.com/questions/3531060/how-to-initialize-a-static-const-member-in-c
        // static const GLint POS_ITEMSIZE = 3;
        // static const GLint AO_ITEMSIZE = 1;
        // static const GLint UV_ITEMSIZE = 2;
        // static const GLint UV_OFFSET_ITEMSIZE = 1;
    public:
        // static const GLint STRIDE = POS_ITEMSIZE + AO_ITEMSIZE + UV_ITEMSIZE + UV_OFFSET_ITEMSIZE;
        static const GLint STRIDE = 2;
        
        Igsi::vec3 coords;
        std::vector<char> data;
        int drawCount;
        
        int numNeighbors;
        int numFilledNeighbors;
        int numPopulatedNeighbors;
        int numGeometryGeneratedNeighbors;

        GLuint VAO;
        GLuint VBO;

        // std::vector<float> geometryData;
        std::vector<GLuint> geometryData;

        Chunk(Igsi::vec3 coords);

        void setVoxel(Igsi::vec3 local, char blockType);
        char getVoxel(Igsi::vec3 local);

        int addCubeFace(int faceId, char blockType, Igsi::vec3 local, char N[3][3][3]);
    };
}

#endif