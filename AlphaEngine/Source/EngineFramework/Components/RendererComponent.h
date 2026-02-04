#pragma once

#include <cstdint>

namespace AlphaEngine {

    struct RenderComponent {
        uint32_t vao;         // The Vertex Array Object ID
        uint32_t shaderID;    // The compiled Shader Program ID
        uint32_t indexCount;  // How many indices to draw (e.g., 3 for a triangle)
        uint32_t textureID;   // Optional: 0 if no texture

        RenderComponent(uint32_t vao = 0, uint32_t shader = 0, uint32_t count = 0)
            : vao(vao), shaderID(shader), indexCount(count), textureID(0) {
        }
    };
}