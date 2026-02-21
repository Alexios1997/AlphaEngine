#pragma once

#include <cstdint>
#include "EngineFramework/AssetManager.h"

namespace AlphaEngine {

    struct RenderComponent {
        AssetHandler meshHandler;    
        AssetHandler shaderHandler;
        AssetHandler textureHandler;
        bool isSkybox = false;
        int layerID = 1;

        RenderComponent() = default;

        RenderComponent(AssetHandler mesh, AssetHandler shader, AssetHandler tex = AssetHandler())
            : meshHandler(mesh), shaderHandler(shader), textureHandler(tex) {
        }
    };
}