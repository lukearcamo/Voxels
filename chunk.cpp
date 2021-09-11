#include "chunk.h"
#include "gen.h"

#include <glad/gl.h>

#include "dependencies/igsi/core/vec3.h"
#include "dependencies/igsi/core/mat4.h"
#include "dependencies/igsi/core/helpers.h"

#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>

using namespace Igsi;

namespace Voxels {
    const vec3 chunkDims = vec3(16, 16, 16);

    const int NUM_VOXELS = chunkDims.x * chunkDims.y * chunkDims.z;
    const int MAX_VERTS = NUM_VOXELS * 6 * 6; // * faces per voxel * verts per face

    const float faceVertexData[6][18] = {
        { 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1 }, // N
        { 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0 }, // S
        { 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0 }, // E
        { 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1 }, // W
        { 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0 }, // T
        { 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1 }, // B
    };
    // We cannot use the flipped uvs because then the AO gets messed up -- instead we flip the uv in the frag shader
    // const float uvData[12] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0 }; // Flipped vertically to make texture right side up
    // const float uvData[12] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1 };
    const char uvData[12] = { 0, 0xf, 0, 0, 0xf, 0xf, 0, 0, 0xf, 0, 0xf, 0xf };

    const int TOTAL_NUM_BLOCK_TYPES = 4;
    const char atlasLUT[TOTAL_NUM_BLOCK_TYPES + 1][6] = {
        //N, S, E, W, T, B
        { 0, 0, 0, 0, 0, 0 }, // air
        { 1, 1, 1, 1, 2, 3 }, // grass
        { 3, 3, 3, 3, 3, 3 }, // dirt
        { 4, 4, 4, 4, 4, 4 }, // stone
        { 6, 6, 6, 6, 6, 6 }  // glass
    };

    Chunk::Chunk(vec3 coords) {
        this->coords = coords;
        data.assign(NUM_VOXELS, 0); // This takes up 4kb so once u get chunks working, you should move to using files
        drawCount = 0;
        
        numNeighbors = 0;
        numFilledNeighbors = 0;
        numPopulatedNeighbors = 0;
        numGeometryGeneratedNeighbors = 0;

        geometryData.reserve(MAX_VERTS * STRIDE);

        VAO = createVAO();
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
                // glBufferData(GL_ARRAY_BUFFER, (MAX_VERTS * STRIDE) * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                // glVertexAttribPointer(0, POS_ITEMSIZE, GL_FLOAT, false, STRIDE * sizeof(float), (void*)0);
                // glEnableVertexAttribArray(0);

                // glVertexAttribPointer(1, AO_ITEMSIZE, GL_FLOAT, false, STRIDE * sizeof(float), (void*)(POS_ITEMSIZE * sizeof(float)));
                // glEnableVertexAttribArray(1);

                // glVertexAttribPointer(2, UV_ITEMSIZE, GL_FLOAT, false, STRIDE * sizeof(float), (void*)((POS_ITEMSIZE + AO_ITEMSIZE) * sizeof(float)));
                // glEnableVertexAttribArray(2);

                // glVertexAttribPointer(3, UV_OFFSET_ITEMSIZE, GL_FLOAT, false, STRIDE * sizeof(float), (void*)((POS_ITEMSIZE + AO_ITEMSIZE + UV_ITEMSIZE) * sizeof(float)));
                // glEnableVertexAttribArray(3);

                glBufferData(GL_ARRAY_BUFFER, (MAX_VERTS * STRIDE) * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

                glVertexAttribPointer(0, 4, GL_INT_2_10_10_10_REV, GL_FALSE, STRIDE * sizeof(GLuint), (void*)0);
                glEnableVertexAttribArray(0);
                
                // ...IPointer, no normalize
                // glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, STRIDE * sizeof(GLuint), (void*)0);
                // glEnableVertexAttribArray(0);

                // ...IPointer, no normalize
                glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, STRIDE * sizeof(GLuint), (void*)(sizeof(GLuint)));
                glEnableVertexAttribArray(1);
    }

    void Chunk::setVoxel(vec3 local, char blockType) {
        data.at(local.x + (local.y * chunkDims.x) + (local.z * chunkDims.x * chunkDims.y)) = blockType;
    }
    char Chunk::getVoxel(vec3 local) {
        return data.at(local.x + (local.y * chunkDims.x) + (local.z * chunkDims.x * chunkDims.y));
    }
    
    int Chunk::addCubeFace(int faceId, char blockType, vec3 local, char N[3][3][3]) {
        bool top, left, bottom, right, topLeft, topRight, bottomLeft, bottomRight;

        switch (faceId) {
            case 0: top = N[1][2][2], left = N[0][1][2], bottom = N[1][0][2], right = N[2][1][2], topLeft = N[0][2][2], topRight = N[2][2][2], bottomLeft = N[0][0][2], bottomRight = N[2][0][2]; break;
            case 1: top = N[1][2][0], left = N[2][1][0], bottom = N[1][0][0], right = N[0][1][0], topLeft = N[2][2][0], topRight = N[0][2][0], bottomLeft = N[2][0][0], bottomRight = N[0][0][0]; break;
            case 2: top = N[2][2][1], left = N[2][1][2], bottom = N[2][0][1], right = N[2][1][0], topLeft = N[2][2][2], topRight = N[2][2][0], bottomLeft = N[2][0][2], bottomRight = N[2][0][0]; break;
            case 3: top = N[0][2][1], left = N[0][1][0], bottom = N[0][0][1], right = N[0][1][2], topLeft = N[0][2][0], topRight = N[0][2][2], bottomLeft = N[0][0][0], bottomRight = N[0][0][2]; break;
            case 4: top = N[1][2][0], left = N[0][2][1], bottom = N[1][2][2], right = N[2][2][1], topLeft = N[0][2][0], topRight = N[2][2][0], bottomLeft = N[0][2][2], bottomRight = N[2][2][2]; break;
            case 5: top = N[1][0][2], left = N[0][0][1], bottom = N[1][0][0], right = N[2][0][1], topLeft = N[0][0][2], topRight = N[2][0][2], bottomLeft = N[0][0][0], bottomRight = N[2][0][0]; break;
        }

        // From https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
        char v00 = top && left ? 0 : (3 - top - left - topLeft);
        char v01 = top && right ? 0 : (3 - top - right - topRight);
        char v10 = bottom && left ? 0 : (3 - bottom - left - bottomLeft);
        char v11 = bottom && right ? 0 : (3 - bottom - right - bottomRight);

        for (int i = 0; i < 6; i++) {
            int e = i * 3;

            // 2_10_10_10 (2 is blank)
            // Map 0~16 to 0~256, leave room from -512~511
            // so that we can fit geometry that sticks out from chunk,
            // while still allowing positions at multiples of 1/16th of a block

            // MB later expand to 11_11_10
            // Idea: If you use unsigned int, maybe you can just add 16 for the sticking-out buffer thingie
            // 16 * 32(more detail) = 512, + 16 = still fits inside 10 bit uint (1024)

            int ix = (faceVertexData[faceId][e] + local.x) * 16.0;
            int iy = (faceVertexData[faceId][e + 1] + local.y) * 16.0;
            int iz = (faceVertexData[faceId][e + 2] + local.z) * 16.0;

            GLuint packedPosition = ix & 0x3ff;
            packedPosition |= (iy & 0x3ff) << 10;
            packedPosition |= (iz & 0x3ff) << 20;

            geometryData.push_back(packedPosition);


            e = i * 2;

            // 4_4_8_2_2_2_2
            // Since these are all chars, all positive integers (we assume char is unsigned)
            // and these are guaranteed to all be within their appropriate ranges,
            // we dont have to do the & operation

            //  pass in offset as (uvx + offset) / 12,
            // To do that, since we cant pass a float, if we do a division by non multiple of two then we lose precision
            // Just make num atlas faces be multiple of two

            GLuint packedUVAO = uvData[e];
            packedUVAO |= uvData[e + 1] << 4;

            packedUVAO |= atlasLUT[blockType][faceId] << 8;

            packedUVAO |= v00 << 16;
            packedUVAO |= v01 << 18;
            packedUVAO |= v10 << 20;
            packedUVAO |= v11 << 24;

            geometryData.push_back(packedUVAO);
        }
        return 6; // Add this to tmpDrawCount
    }
}