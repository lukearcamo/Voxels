#include "geometry.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"

#include <vector>
#include <map>
#include <cmath>

namespace Igsi {
    // These functions are not meant to be used outside of this source, so they are not declared in the header
    void setXYZ(std::vector<float> &buffer, int idx, vec3 value) {
        buffer.at(idx) = value.x;
        buffer.at(idx + 1) = value.y;
        buffer.at(idx + 2) = value.z;
    }
    vec3 getXYZ(std::vector<float> &buffer, int idx) {
        return vec3(buffer.at(idx), buffer.at(idx + 1), buffer.at(idx + 2));
    }

    std::vector<float>& Geometry::addAttribute(std::string key, int itemSize) {
        itemSizes.emplace(key, itemSize);
        return attributes.emplace(key, std::vector<float>()).first->second;
    }
    std::vector<float>& Geometry::getAttribute(std::string key) {
        return attributes.at(key);
    }
    void Geometry::deleteAttribute(std::string key) { // https://stackoverflow.com/questions/23022622/vector-inside-a-map-is-clear-safe
        attributes.erase(key);
        itemSizes.erase(key);
    }

    int Geometry::getDrawCount() {
        int idcs = indices.size();
        return idcs > 0 ? idcs : (attributes.at("position").size() / itemSizes.at("position"));
    }
    void Geometry::toNonIndexed() {
        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
            int itemSize = itemSizes.at(it->first);
            std::vector<float> newBuffer;
            newBuffer.reserve(indices.size() * itemSize);

            for (int i = 0; i < indices.size(); i++) {
                int idx = indices.at(i) * itemSize;
                for (int j = 0; j < itemSize; j++) {
                    newBuffer.push_back(it->second.at(idx + j));
                }
            }
            it->second.swap(newBuffer);
        }
        indices.clear();
        indices.shrink_to_fit();
    }

    void Geometry::transform(mat4 matrix) {
        std::vector<float> &vertices = addAttribute("position", 3);
        std::vector<float> &normals = addAttribute("normal", 3);

        mat4 normalMatrix = matrix;
        normalMatrix.invert().transpose(); // Assumes elements 3, 7, and 11 are 0, which should be true if using the regular matrix transformations defined in matrix class
        for (int i = 0; i < vertices.size(); i += 3) {
            vec3 tmp = getXYZ(vertices, i);
            vec4 pos = matrix * vec4(tmp.x, tmp.y, tmp.z, 1);

            tmp = getXYZ(normals, i);
            vec4 nml = normalMatrix * vec4(tmp.x, tmp.y, tmp.z, 0);

            setXYZ(vertices, i, vec3(pos.x, pos.y, pos.z));
            setXYZ(normals, i, vec3(nml.x, nml.y, nml.z));
        }
    }
    void Geometry::computeNormals() {
        std::vector<float> &vertices = addAttribute("position", 3);
        std::vector<float> &normals = addAttribute("normal", 3);

        normals.assign(vertices.size(), 0);

        if (indices.size() == 0) {
            for (int i = 0; i < vertices.size(); i += 9) {
                vec3 pA = getXYZ(vertices, i);
                vec3 pB = getXYZ(vertices, i + 3);
                vec3 pC = getXYZ(vertices, i + 6);
                vec3 nml = normalize(cross(pC - pB, pA - pB));
                setXYZ(normals, i, nml);
                setXYZ(normals, i + 3, nml);
                setXYZ(normals, i + 6, nml);
            }
        }
        else {
            for (int i = 0; i < indices.size(); i += 3) {
                vec3 index = vec3(indices.at(i), indices.at(i + 1), indices.at(i + 2));
                index *= 3; // Because every vertex is three components in the actual position buffer
                vec3 pA = getXYZ(vertices, index.x);
                vec3 pB = getXYZ(vertices, index.y);
                vec3 pC = getXYZ(vertices, index.z);
                vec3 nml = cross(pC - pB, pA - pB);
                setXYZ(normals, index.x, getXYZ(normals, index.x) + nml);
                setXYZ(normals, index.y, getXYZ(normals, index.y) + nml);
                setXYZ(normals, index.z, getXYZ(normals, index.z) + nml);
            }
            for (int i = 0; i < normals.size(); i += 3) {
                setXYZ(normals, i, normalize(getXYZ(normals, i)));
            }
        }
    }

    void planeGeometry(Geometry* geometry, vec2 dims, vec2 segments) {
        vec2 halfDims = dims / 2;
		vec2 grid = floor(segments);
		vec2 grid1 = grid + 1;
		vec2 segmentSize = dims / grid;

        std::vector<float> &vertices = geometry->addAttribute("position", 3);
        std::vector<float> &normals = geometry->addAttribute("normal", 3);
        std::vector<float> &uvs = geometry->addAttribute("uv", 2);
        std::vector<unsigned int> &indices = geometry->indices;
        
        int pastSize = vertices.size() / 3;

        int numVerts = grid1.x * grid1.y;
        int numIdcs = grid.x * grid.y * 6;
        vertices.reserve(vertices.size() + numVerts * 3);
        normals.reserve(normals.size() + numVerts * 3);
        uvs.reserve(uvs.size() + numVerts * 2);
        indices.reserve(indices.size() + numIdcs);

		for (int iy = 0; iy < grid1.y; iy++) {
			float y = iy * segmentSize.y - halfDims.y;
			for (int ix = 0; ix < grid1.x; ix++) {
				float x = ix * segmentSize.x - halfDims.x;
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(0);

				normals.push_back(0);
				normals.push_back(0);
				normals.push_back(1);
                
                uvs.push_back(ix / grid.x);
				uvs.push_back(1.0 - (iy / grid.y)); // So that y=0 is on bottom rather than top;
			}
		}
		for (int iy = 0; iy < grid.y; iy++) {
			for (int ix = 0; ix < grid.x; ix++) {
                unsigned int a = grid1.x * iy + ix + pastSize;
				unsigned int b = grid1.x * (iy + 1) + ix + pastSize;
				unsigned int c = grid1.x * iy + (ix + 1) + pastSize;
				unsigned int d = grid1.x * (iy + 1) + (ix + 1) + pastSize;
                
				indices.push_back(a);
				indices.push_back(d);
				indices.push_back(b);
				indices.push_back(a);
				indices.push_back(c);
				indices.push_back(d);
			}
		}
    }
    // Adapted from BoxGeometry in THREE.js library
    // https://github.com/mrdoob/three.js/blob/master/src/geometries/BoxGeometry.js
    void boxGeometry(Geometry* geometry, vec3 dims, vec3 segments) {
        vec3 halfDims = dims / 2;

        std::vector<float> &vertices = geometry->addAttribute("position", 3);
        std::vector<float> &normals = geometry->addAttribute("normal", 3);
        std::vector<float> &uvs = geometry->addAttribute("uv", 2);
        std::vector<unsigned int> &indices = geometry->indices;

        int pastSize = vertices.size() / 3;

        auto face = [&](int u, int v, int w, int udir, int vdir, vec3 relDims) {
            vec2 grid = floor(vec2(segments[u], segments[v]));
            vec2 grid1 = grid + 1;
            vec2 segmentSize = vec2(dims[u], dims[v]) / grid;

            vec3 tmp;
            
            int numVerts = grid1.y * grid1.x;
            int numIdcs = grid.y * grid.x * 6;
            vertices.reserve(vertices.size() + numVerts * 3);
            normals.reserve(normals.size() + numVerts * 3);
            uvs.reserve(uvs.size() + numVerts * 2);
            indices.reserve(indices.size() + numIdcs);

            for (int iy = 0; iy < grid1.y; iy++) {
                float y = iy * segmentSize.y - relDims.y;
                for (int ix = 0; ix < grid1.x; ix++) {
                    float x = ix * segmentSize.x - relDims.x;

                    tmp[u] = x * udir;
                    tmp[v] = y * vdir;
                    tmp[w] = relDims.z;
                    
                    vertices.push_back(tmp.x);
                    vertices.push_back(tmp.y);
                    vertices.push_back(tmp.z);

                    tmp[u] = 0;
                    tmp[v] = 0;
                    tmp[w] = relDims.z > 0 ? 1 : -1;

                    normals.push_back(tmp.x);
                    normals.push_back(tmp.y);
                    normals.push_back(tmp.z);
                    
                    uvs.push_back(ix / grid.x);
                    uvs.push_back(iy / grid.y);
                }
            }
            for (int iy = 0; iy < grid.y; iy++) {
                for (int ix = 0; ix < grid.x; ix++) {
                    unsigned int a = grid1.x * iy + ix + pastSize;
                    unsigned int b = grid1.x * iy + (ix + 1) + pastSize;
                    unsigned int c = grid1.x * (iy + 1) + ix + pastSize;
                    unsigned int d = grid1.x * (iy + 1) + (ix + 1) + pastSize;
                    
                    indices.push_back(a);
                    indices.push_back(c);
                    indices.push_back(b);
                    indices.push_back(c);
                    indices.push_back(d);
                    indices.push_back(b);
                }
            }

            pastSize += numVerts;
        };

        face(2, 1, 0, -1, -1, vec3(halfDims.z, halfDims.y, halfDims.x)); // px
		face(2, 1, 0, 1, -1, vec3(halfDims.z, halfDims.y, -halfDims.x)); // nx
		face(0, 2, 1, 1, 1, vec3(halfDims.x, halfDims.z, halfDims.y)); // py
		face(0, 2, 1, 1, -1, vec3(halfDims.x, halfDims.z, -halfDims.y)); // ny
		face(0, 1, 2, 1, -1, vec3(halfDims.x, halfDims.y, halfDims.z)); // pz
		face(0, 1, 2, -1, -1, vec3(halfDims.x, halfDims.y, -halfDims.z)); // nz
    }
    // Adapted from Song Ho's article on OpenGL Spheres and SphereGeometry in THREE.js library
    // http://www.songho.ca/opengl/gl_sphere.html
    // https://github.com/mrdoob/three.js/blob/master/src/geometries/SphereGeometry.js
    void sphereGeometry(Geometry* geometry, float radius, vec2 segments) {
        std::vector<float> &vertices = geometry->addAttribute("position", 3);
        std::vector<float> &normals = geometry->addAttribute("normal", 3);
        std::vector<float> &uvs = geometry->addAttribute("uv", 2);
        std::vector<unsigned int> &indices = geometry->indices;
        
        int pastSize = vertices.size() / 3;

        int numVerts = (segments.y + 1) * (segments.x + 1);
        int numIdcs = segments.y * segments.x * 6 - segments.x * 2;
        vertices.reserve(vertices.size() + numVerts * 3);
        normals.reserve(normals.size() + numVerts * 3);
        uvs.reserve(uvs.size() + numVerts * 2);
        indices.reserve(indices.size() + numIdcs);

        for (int iy = 0; iy <= segments.y; iy++) {
            float v = iy / segments.y;
            float phi = v * M_PI;
            for (int ix = 0; ix <= segments.x; ix++) {
                float u = ix / segments.x;
                float theta = u * M_PI * 2;

                vec3 p = vec3( // Normalized
                    -std::cos(theta) * std::sin(phi),
				    std::cos(phi),
				    std::sin(theta) * std::sin(phi)
                );
                
				normals.push_back(p.x);
                normals.push_back(p.y);
                normals.push_back(p.z);

                vertices.push_back(p.x * radius);
                vertices.push_back(p.y * radius);
                vertices.push_back(p.z * radius);

                uvs.push_back(u);
                uvs.push_back(v);
            }
        }
        for (int iy = 0; iy < segments.y; iy++) {
			for (int ix = 0; ix < segments.x; ix++) {
                unsigned int a = (segments.x + 1) * iy + ix + pastSize;
                unsigned int b = (segments.x + 1) * iy + (ix + 1) + pastSize;
                unsigned int c = (segments.x + 1) * (iy + 1) + ix + pastSize;
                unsigned int d = (segments.x + 1) * (iy + 1) + (ix + 1) + pastSize;

                if (iy != 0) {
                    indices.push_back(a);
                    indices.push_back(c);
                    indices.push_back(b);
                }
                if (iy != segments.y - 1) {
                    indices.push_back(c);
                    indices.push_back(d);
                    indices.push_back(b);
                }
			}
		}
    }
}