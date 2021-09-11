#ifndef IGSI_GEOMETRY_H
#define IGSI_GEOMETRY_H

#include "vec2.h"
#include "vec3.h"

#include <vector>
#include <map>
#include <string>

namespace Igsi {
    // class vec2; // Even if you declare again it wont overwrite existing declaration
    class mat4;

    class Geometry {
    public:
        std::map<std::string, std::vector<float>> attributes;
        std::map<std::string, int> itemSizes;
        std::vector<unsigned int> indices;

        std::vector<float> &addAttribute(std::string key, int itemSize); // If attribute name already exists, addAttribute will not add another one
        std::vector<float> &getAttribute(std::string key);
        void deleteAttribute(std::string key);

        int getDrawCount();
        void toNonIndexed();

        void transform(mat4 matrix);
        void computeNormals();
    };

    void planeGeometry(Geometry* geometry, vec2 dims=vec2(1), vec2 segments=vec2(1));
    void boxGeometry(Geometry* geometry, vec3 dims=vec3(1), vec3 segments=vec3(1));
    void testing(Igsi::Geometry* geometry, std::vector<float>* asdasd);
    void sphereGeometry(Geometry* geometry, float radius=1.0, vec2 segments=vec2(8, 6));
}

#endif