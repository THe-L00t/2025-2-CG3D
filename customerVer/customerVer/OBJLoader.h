#pragma once

// DLL Export/Import 매크로
#ifdef LOBJLOADERT_EXPORTS
    #define OBJLOADER_API __declspec(dllexport)
#else
    #define OBJLOADER_API __declspec(dllimport)
    // 자동으로 Import 라이브러리 링크 (Visual Studio 설정 불필요)
    #pragma comment(lib, "L_ObjLoader_t.lib")
#endif

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct OBJLOADER_API Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class OBJLOADER_API OBJLoader {
public:
    static bool Load(const std::string& path,
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices);
};
