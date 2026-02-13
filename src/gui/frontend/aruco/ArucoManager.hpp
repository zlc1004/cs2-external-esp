#pragma once

#include <vector>
#include <string>
#include <d3d11.h>
#include "../../../external/imgui/imgui.h"
#include "../../../external/AsyncLogger/include/AsyncLogger/Logger.hpp"

// stb_image for PNG loading
#define STB_IMAGE_IMPLEMENTATION
#include "../../../external/stb/stb_image.h"

// Forward declare Window's device
class Window;

class ArucoManager {
public:
    ~ArucoManager() = default;
    ArucoManager(const ArucoManager&) = delete;
    ArucoManager(ArucoManager&&) = delete;
    ArucoManager& operator=(const ArucoManager&) = delete;
    ArucoManager& operator=(ArucoManager&&) = delete;

    static bool Init(ID3D11Device* device) {
        return GetInstance().InitImpl(device);
    }
    
    static void Destroy() {
        return GetInstance().DestroyImpl();
    }
    
    static ImTextureID GetMarker(int index) {
        auto& instance = GetInstance();
        if (index < 0 || index >= static_cast<int>(instance.textures.size())) {
            return nullptr;
        }
        return instance.textures[index];
    }
    
    static int GetMarkerCount() {
        return MARKER_COUNT;
    }

private:
    ArucoManager() = default;

    static ArucoManager& GetInstance() {
        static ArucoManager instance;
        return instance;
    }

    bool InitImpl(ID3D11Device* device) {
        textures.reserve(MARKER_COUNT);
        
        for (int i = 1; i <= MARKER_COUNT; ++i) {
            std::string path = "aruco/4x4_1000-" + std::to_string(i) + ".png";
            ID3D11ShaderResourceView* texture = LoadTextureFromFile(device, path);
            textures.push_back(texture);
            
            if (texture) {
                LOGF(al::INFO, "Loaded ArUco marker {} from {}", i, path);
            } else {
                LOGF(al::WARNING, "Failed to load ArUco marker {} from {}", i, path);
            }
        }
        
        return true;
    }
    
    void DestroyImpl() {
        for (auto& texture : textures) {
            if (texture) {
                texture->Release();
                texture = nullptr;
            }
        }
        textures.clear();
    }

    ID3D11ShaderResourceView* LoadTextureFromFile(ID3D11Device* device, const std::string& filename) {
        int width, height, channels;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4); // Force RGBA
        
        if (!data) {
            return nullptr;
        }
        
        // Create texture
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        
        D3D11_SUBRESOURCE_DATA subResource = {};
        subResource.pSysMem = data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        
        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = device->CreateTexture2D(&desc, &subResource, &texture);
        
        if (FAILED(hr)) {
            stbi_image_free(data);
            return nullptr;
        }
        
        // Create shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        
        ID3D11ShaderResourceView* textureView = nullptr;
        hr = device->CreateShaderResourceView(texture, &srvDesc, &textureView);
        
        texture->Release();
        stbi_image_free(data);
        
        if (FAILED(hr)) {
            return nullptr;
        }
        
        return textureView;
    }

    std::vector<ID3D11ShaderResourceView*> textures;
    static constexpr int MARKER_COUNT = 13;
};
