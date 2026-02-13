#pragma once

#include <vector>
#include <string>
#include "../../../external/imgui/imgui.h"
#include "../../../external/AsyncLogger/include/AsyncLogger/Logger.hpp"

#ifdef _WIN32
#include <d3d11.h>
#else
// Dummy definitions for non-Windows compilation
// These will allow the code to compile on macOS but will not provide actual functionality
struct ID3D11Device {};
struct ID3D11ShaderResourceView {
    void Release() { /* do nothing */ }
};

struct D3D11_TEXTURE2D_DESC {
    unsigned int Width;
    unsigned int Height;
    unsigned int MipLevels;
    unsigned int ArraySize;
    unsigned int Format;
    struct {
        unsigned int Count;
        unsigned int Quality;
    } SampleDesc;
    unsigned int Usage;
    unsigned int BindFlags;
    unsigned int CPUAccessFlags;
    unsigned int MiscFlags;
};

struct D3D11_SUBRESOURCE_DATA {
    const void* pSysMem;
    unsigned int SysMemPitch;
    unsigned int SysMemSlicePitch;
};

struct ID3D11Texture2D {
    void Release() { /* do nothing */ }
};

using HRESULT = int;
#define DXGI_FORMAT_R8G8B8A8_UNORM 0
#define D3D11_USAGE_DEFAULT 0
#define D3D11_BIND_SHADER_RESOURCE 0
#define D3D11_SRV_DIMENSION_TEXTURE2D 0
#define FAILED(hr) (hr < 0)

#endif

// stb_image for PNG loading
#include "../../../external/stb/stb_image.h"

// Forward declare Window's device
class Window;

// Forward declare stb_image functions (defined in Renderer.cpp with STB_IMAGE_IMPLEMENTATION)
unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
void stbi_image_free(void* retval_from_stbi_load);


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
            return (ImTextureID)0;
        }
        return (ImTextureID)instance.textures[index];
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
#ifdef _WIN32
                texture->Release();
#endif
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
        
        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = 0; // Initialize HRESULT for macOS

#ifdef _WIN32
        D3D11_SUBRESOURCE_DATA subResource = {};
        subResource.pSysMem = data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        hr = device->CreateTexture2D(&desc, &subResource, &texture);
#endif
        
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
#ifdef _WIN32
        hr = device->CreateShaderResourceView(texture, &srvDesc, &textureView);
#endif
        
        #ifdef _WIN32
        texture->Release();
        #endif
        stbi_image_free(data);
        
        if (FAILED(hr)) {
            return nullptr;
        }
        
        return textureView;
    }

    std::vector<ID3D11ShaderResourceView*> textures;
    static constexpr int MARKER_COUNT = 13;
};
