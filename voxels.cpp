#include <glad/gl.h>
// #define GLFW_DLL
#define GLFW_INCLUDE_NONE // Just to be sure
#include <GLFW/glfw3.h>

#include "dependencies/igsi/core/vec2.h"
#include "dependencies/igsi/core/vec3.h"
#include "dependencies/igsi/core/mat4.h"
#include "dependencies/igsi/core/transform.h"
#include "dependencies/igsi/core/helpers.h"
#include "dependencies/igsi/core/geometry.h"

#include "dependencies/igsi/extra/controls.h"
#include "dependencies/igsi/extra/imageLoader.h"
#include "dependencies/igsi/extra/skybox.h"
#include "dependencies/igsi/extra/text.h"

#include "chunk.h"
#include "chunkManager.h"
#include "gen.h"
#include "chunkUpdater.h"
#include "frustum.h"


#include <iostream>
#include <thread>
// #include <vector>
#include <chrono>

#include <sstream>
#include <iomanip>

#define PI 3.14159265358979323846
#define TO_RADIANS PI / 180.0

using namespace Igsi;

namespace Voxels {
    GLFWwindow* window;
    float width = 960.0, height = 540.0;
    bool hasResized = false;
    void onResize(GLFWwindow* window, int w, int h) { width = w; height = h; hasResized = true; }
    void onError(int error, const char* description) { std::cerr << "Error" << error << ": " << description << "\n"; }
    
    int mouseBtn = -1;
    void onClick(GLFWwindow* window, int button, int action, int mods) {
        Controls::mouseEvent(window, button, action, mods);
        if (action == GLFW_PRESS) mouseBtn = button;
        else if (action == GLFW_RELEASE) mouseBtn = -1;
    }

    int currentBlockId = 0;
    void onScroll(GLFWwindow* window, double xoffset, double yoffset) {
        currentBlockId += (yoffset > 0) - (yoffset < 0); // Sign function
        currentBlockId = ((currentBlockId % TOTAL_NUM_BLOCK_TYPES) + TOTAL_NUM_BLOCK_TYPES) % TOTAL_NUM_BLOCK_TYPES; // Mod function 
        std::cout << "currentBlockId: " << currentBlockId << std::endl;
    }
    
    int init() {
        glfwSetErrorCallback(onError);
        if (!glfwInit()) {
            std::cerr << "GLFW initialization failed\n";
            return -1;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(width, height, "Fenetre", NULL, NULL);
        if (!window) {
            std::cerr << "Window creation failed\n";
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window); // GLAD requires active context
        // if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        if (!gladLoadGL(glfwGetProcAddress)) {
            std::cerr << "GLAD initialization failed\n";
            return -1;
        }
        glfwMakeContextCurrent(NULL); // Will set context on the other thread
        return 0;
    }

    ChunkManager chunkManager;
    ChunkGenerator chunkGenerator; // MB different instances for different terrain parameters
    ChunkUpdater chunkUpdater(&chunkManager, &chunkGenerator);

    // Next steps: Try a rudimentary test for flood fillling
    // If it is still performant enough, you can try minecraft's cave culling algo
    // Related: portal rendering / Portal culling
    // Related: https://www.youtube.com/watch?v=UMYFEYju40k


    // These threads ait a bit until next loop iteration so cpu doesnt overwork itself by running this loop at warp speed
    // MB decrease waiting time a bit

    void chunkFillThread() {
        while (!glfwWindowShouldClose(window)) {
            chunkUpdater.fillNext();
        }
    }
    void chunkPopulateThread() {
        while (!glfwWindowShouldClose(window)) {
            chunkUpdater.populateNext();
        }
    }
    void chunkGeoThread() {
        while (!glfwWindowShouldClose(window)) {
            chunkUpdater.buildNext();
        }
    }

    void render() {
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, onResize);

        // ======= Setup =======

        Transform camera;
        mat4 projectionMatrix = mat4().perspective(60 * TO_RADIANS, width / height, 0.1, 1000);
        Frustum frustum(60 * TO_RADIANS, width / height, 0.1, 1000);

        glfwSetKeyCallback(window, Controls::keyEvent);
        glfwSetMouseButtonCallback(window, onClick);
        glfwSetWindowFocusCallback(window, Controls::focusEvent);
        glfwSetScrollCallback(window, onScroll);

        // ======= Skybox -- texUnit 0 =======

        Skybox skybox(0, "./textures/sky/sunset/");
        
        // ======= Atlas texture -- texUnit 1 =======

        GLuint atlasTex = createTexture(GL_TEXTURE_2D, 1);
        setTexParams(GL_TEXTURE_2D, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        loadImage("./textures/atlas/atlas.png", GL_TEXTURE_2D);

        // ======= Font -- texUnit 2 =======

        GLuint fontTex = createTexture(GL_TEXTURE_2D, 2);
        setTexParams(GL_TEXTURE_2D, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        vec2 fontTexDims = loadImage("./textures/fonts/consolas.png", GL_TEXTURE_2D);

        Text debugText(30, vec2(-0.99, 1), vec2(0.1), vec3(1.0), Text::LEFT);
        debugText.setFontTexture(2, fontTexDims);

        // ======= Chunks =======

        GLuint chunkProgram = createShaderProgram(readFile("./shaders/voxels.vert"), readFile("./shaders/voxels.frag"));
        setUniformInt("fogMap", 0);
        setUniformInt("map", 1);

        // horiz range = 3 + 3 + 1 = 7 chunks
        // vert range = -5, -4, -3, -2, -1 = 5 chunks
        // volume = 7 * 7 * 5 = 245 chunks
        // minecraft normal render distance 8 = 8 + 8 + 1 = 17 chunks

        // const int renderDistance = 6;
        // const int renderDistance = 5;
        const int renderDistance = 3;

        for (int x = -renderDistance; x <= renderDistance; x++) {
            for (int y = -5; y < 0; y++) {
                for (int z = -renderDistance; z <= renderDistance; z++) {
                    Chunk &ch = chunkManager.addChunk(vec3(x, y, z));
                    chunkUpdater.fillQueue.push_back(ChunkManager::coordsToId(vec3(x, y, z))); // MB later on put this directly inside addChunk?
                }
            }
        }
        // MB set numNeighbors of neighboring chunks in addChunk
        // rather than in fill data

        // ======= Selection =======
        
        std::string basic_frag = readFile("./shaders/basic.frag");

        GLuint crosshairProgram = createShaderProgram(readFile("./shaders/crosshair.vert"), basic_frag);
        setUniform("color", vec3(0.0, 1.0, 0.0));

        GLuint boxWireframeProgram = createShaderProgram(readFile("./shaders/basic.vert"), basic_frag);

        // Notice how the box is corner-aligned
        // TODO: Convert to float literals
        GLfloat boxWireframeVerts[72] = {
            1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0,
            0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1,
            0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1,
            0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0,
            0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1,
            1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0
        };
        GLuint boxWireframeIndices[24] = { 0, 1, 2, 3, 0, 2, 1, 3, 4, 5, 6, 7, 4, 6, 5, 7, 0, 5, 1, 4, 2, 7, 3, 6 };

        GLuint boxWireframeVAO = createVAO();
            createVBO(0, boxWireframeVerts, sizeof(boxWireframeVerts), 3, GL_FLOAT);

        Transform selection;

        mat4 chunkBoundsWorldMatrix;
        chunkBoundsWorldMatrix.scale(chunkDims);
        
        vec3 regionDims = vec3(16, 16, 16) * chunkDims; // TMP
        mat4 regionBoundsWorldMatrix;
        regionBoundsWorldMatrix.scale(regionDims);

        // ======= Render =======

        // glPrimitiveRestartIndex
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glLineWidth(2);
        glfwSwapInterval(1);
        float prevFrameTime = glfwGetTime();

        while(!glfwWindowShouldClose(window)) {
            float now = glfwGetTime();
            float deltaTime = now - prevFrameTime;
            prevFrameTime = now;

            float aspect = width / height;

            if (hasResized) {
                glViewport(0, 0, width, height);
                projectionMatrix.perspective(60 * TO_RADIANS, aspect, 0.1, 1000);
                frustum.updateAspect(aspect);
                hasResized = false;
            }
            
            vec3 ro = camera.position;
            vec3 rd = -camera.getWorldDirection();

            vec3 playerVoxel = floor(camera.position);
            vec3 c = ChunkManager::getChunkCoords(playerVoxel);

            vec3 selectionNormal;
            chunkManager.raycastVoxels(ro, rd, 5.0, selection.position, selectionNormal);
            if (selectionNormal == vec3(0.0)) {
                selection.position = vec3(0, 1e6, 0);
            }
            else {
                if (mouseBtn == GLFW_MOUSE_BUTTON_LEFT || mouseBtn == GLFW_MOUSE_BUTTON_RIGHT) {
                    char blockId = 0;
                    if (mouseBtn == GLFW_MOUSE_BUTTON_RIGHT) { // If placing block
                        selection.position += selectionNormal;
                        // if (playerVoxel == selection.position) return;
                        blockId = currentBlockId + 1; // + 1 to skip air
                    }

                    vec3 coords = ChunkManager::getChunkCoords(selection.position);
                    vec3 local = ChunkManager::getLocalCoords(selection.position);

                    int oldNumChunks = chunkManager.chunks.size();
                    chunkManager.setVoxelGlobal(selection.position, blockId);
                    int newNumChunks = chunkManager.chunks.size();
                    if (newNumChunks > oldNumChunks) std::cout << "Num Chunks: " << newNumChunks << std::endl;

                    chunkUpdater.buildQueue.push(ChunkManager::coordsToId(coords));
                    if (local.x == 0 || local.y == 0 || local.z == 0 || local.x == 15 || local.y == 15 || local.z == 15) {
                        chunkUpdater.rebuildNeighborChunks(coords, local);
                    }
                    // mouseBtn = -1;
                }
            }

            chunkUpdater.mapNextAll();
            
            Controls::update(window, &camera, deltaTime, 12, 25, 0.001);
            camera.updateMatrices();
            selection.updateMatrices();


            glClearColor(0.25, 0.25, 0.25, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(chunkProgram);
            chunkManager.drawChunks(&camera, projectionMatrix, &frustum);

            glUseProgram(boxWireframeProgram);
            glBindVertexArray(boxWireframeVAO);

            selection.setDefaultUniforms(&camera, projectionMatrix);
            setUniform("color", vec3(1.0));
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boxWireframeIndices);

            chunkBoundsWorldMatrix.setTranslation(floor(camera.position / chunkDims) * chunkDims);
            setUniform("worldMatrix", chunkBoundsWorldMatrix);
            setUniform("color", vec3(1.0, 1.0, 0.0));
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boxWireframeIndices);
            
            regionBoundsWorldMatrix.setTranslation(floor(camera.position / regionDims) * regionDims);
            setUniform("worldMatrix", regionBoundsWorldMatrix);
            setUniform("color", vec3(0.0, 0.0, 1.0));
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boxWireframeIndices);

            skybox.draw(&camera, projectionMatrix);
            
            glDisable(GL_DEPTH_TEST);
            glUseProgram(crosshairProgram);
            glDrawArrays(GL_LINES, 0, 4);
            glEnable(GL_DEPTH_TEST);

            // "xyz: xxx.xx, yyy.yy, zzz.zz" : ~27 chars, just round it to 30
            std::ostringstream ss;
            // ss << "xyz: " << std::fixed << std::setprecision(2) << camera.position.x << ", " << camera.position.y << ", " << camera.position.z;
            ss << "xyz: " << std::fixed << std::setprecision(2) << camera.position;

            debugText.scale = 50.0 / height;
            // 25px absolute size, since scale is relative to -1~1 screnspace, not whole window,
            // and both scale x & y are relative to height
            debugText.updateText(ss.str());
            debugText.draw(aspect);

            glfwSwapBuffers(window);
        }

        // MB do some OpenGL clean up here, before this thread returns? 
    }
}
int main() {
    if (Voxels::init()) return -1;

    std::thread thread1(Voxels::render);
    std::thread thread2(Voxels::chunkFillThread);
    std::thread thread3(Voxels::chunkPopulateThread);
    std::thread thread4(Voxels::chunkGeoThread);

    while(!glfwWindowShouldClose(Voxels::window)) {
        glfwWaitEvents();
    }

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    glfwTerminate();
    return 0;
}