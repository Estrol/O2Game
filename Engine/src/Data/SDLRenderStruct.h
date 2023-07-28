#pragma once
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
    This is combined version to able get private data
*/

#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_rect.h>

#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3d11_1.h>

#define SDL_D3D12_NUM_BUFFERS        2
#define SDL_D3D12_NUM_VERTEX_BUFFERS 256
#define SDL_D3D12_MAX_NUM_TEXTURES   16384
#define SDL_D3D12_NUM_UPLOAD_BUFFERS 32
#define SDL_HAVE_YUV 1

typedef struct
{
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        } v;
        float m[4][4];
    };
} Float4X4;

typedef struct
{
    Float4X4 model;
    Float4X4 projectionAndView;
} VertexShaderConstants;

typedef enum
{
    SHADER_SOLID,
    SHADER_RGB,
#if SDL_HAVE_YUV
    SHADER_YUV_JPEG,
    SHADER_YUV_BT601,
    SHADER_YUV_BT709,
    SHADER_NV12_JPEG,
    SHADER_NV12_BT601,
    SHADER_NV12_BT709,
    SHADER_NV21_JPEG,
    SHADER_NV21_BT601,
    SHADER_NV21_BT709,
#endif
    NUM_SHADERS
} D3D12_Shader;

typedef struct
{
    D3D12_Shader shader;
    SDL_BlendMode blendMode;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
    DXGI_FORMAT rtvFormat;
    ID3D12PipelineState* pipelineState;
} D3D12_PipelineState;

typedef struct
{
    ID3D12Resource* resource;
    D3D12_VERTEX_BUFFER_VIEW view;
    size_t size;
} D3D12_VertexBuffer;

typedef struct
{
    ID3D12Resource* mainTexture;
    D3D12_CPU_DESCRIPTOR_HANDLE mainTextureResourceView;
    D3D12_RESOURCE_STATES mainResourceState;
    SIZE_T mainSRVIndex;
    D3D12_CPU_DESCRIPTOR_HANDLE mainTextureRenderTargetView;
    DXGI_FORMAT mainTextureFormat;
    ID3D12Resource* stagingBuffer;
    D3D12_RESOURCE_STATES stagingResourceState;
    D3D12_FILTER scaleMode;
#if SDL_HAVE_YUV
    /* YV12 texture support */
    SDL_bool yuv;
    ID3D12Resource* mainTextureU;
    D3D12_CPU_DESCRIPTOR_HANDLE mainTextureResourceViewU;
    D3D12_RESOURCE_STATES mainResourceStateU;
    SIZE_T mainSRVIndexU;
    ID3D12Resource* mainTextureV;
    D3D12_CPU_DESCRIPTOR_HANDLE mainTextureResourceViewV;
    D3D12_RESOURCE_STATES mainResourceStateV;
    SIZE_T mainSRVIndexV;

    /* NV12 texture support */
    SDL_bool nv12;
    ID3D12Resource* mainTextureNV;
    D3D12_CPU_DESCRIPTOR_HANDLE mainTextureResourceViewNV;
    D3D12_RESOURCE_STATES mainResourceStateNV;
    SIZE_T mainSRVIndexNV;

    Uint8* pixels;
    int pitch;
#endif
    SDL_Rect lockedRect;
} D3D12_TextureData;

typedef struct
{
    SIZE_T index;
    void* next;
} D3D12_SRVPoolNode;

typedef struct
{
    void* hDXGIMod;
    void* hD3D12Mod;
#if defined(__XBOXONE__) || defined(__XBOXSERIES__)
    UINT64 frameToken;
#else
    IDXGIFactory6* dxgiFactory;
    IDXGIAdapter4* dxgiAdapter;
    IDXGIDebug* dxgiDebug;
    IDXGISwapChain4* swapChain;
#endif
    ID3D12Device1* d3dDevice;
    ID3D12Debug* debugInterface;
    ID3D12CommandQueue* commandQueue;
    ID3D12GraphicsCommandList2* commandList;
    DXGI_SWAP_EFFECT swapEffect;
    UINT swapFlags;

    /* Descriptor heaps */
    ID3D12DescriptorHeap* rtvDescriptorHeap;
    UINT rtvDescriptorSize;
    ID3D12DescriptorHeap* textureRTVDescriptorHeap;
    ID3D12DescriptorHeap* srvDescriptorHeap;
    UINT srvDescriptorSize;
    ID3D12DescriptorHeap* samplerDescriptorHeap;
    UINT samplerDescriptorSize;

    /* Data needed per backbuffer */
    ID3D12CommandAllocator* commandAllocators[SDL_D3D12_NUM_BUFFERS];
    ID3D12Resource* renderTargets[SDL_D3D12_NUM_BUFFERS];
    UINT64 fenceValue;
    int currentBackBufferIndex;

    /* Fences */
    ID3D12Fence* fence;
    HANDLE fenceEvent;

    /* Root signature and pipeline state data */
    ID3D12RootSignature* rootSignatures[4];
    int pipelineStateCount;
    D3D12_PipelineState* pipelineStates;
    D3D12_PipelineState* currentPipelineState;

    D3D12_VertexBuffer vertexBuffers[SDL_D3D12_NUM_VERTEX_BUFFERS];
    D3D12_CPU_DESCRIPTOR_HANDLE nearestPixelSampler;
    D3D12_CPU_DESCRIPTOR_HANDLE linearSampler;

    /* Data for staging/allocating textures */
    ID3D12Resource* uploadBuffers[SDL_D3D12_NUM_UPLOAD_BUFFERS];
    int currentUploadBuffer;

    /* Pool allocator to handle reusing SRV heap indices */
    D3D12_SRVPoolNode* srvPoolHead;
    D3D12_SRVPoolNode srvPoolNodes[SDL_D3D12_MAX_NUM_TEXTURES];

    /* Vertex buffer constants */
    VertexShaderConstants vertexShaderConstantsData;

    /* Cached renderer properties */
    DXGI_MODE_ROTATION rotation;
    D3D12_TextureData* textureRenderTarget;
    D3D12_CPU_DESCRIPTOR_HANDLE currentRenderTargetView;
    D3D12_CPU_DESCRIPTOR_HANDLE currentShaderResource;
    D3D12_CPU_DESCRIPTOR_HANDLE currentSampler;
    SDL_bool cliprectDirty;
    SDL_bool currentCliprectEnabled;
    SDL_Rect currentCliprect;
    SDL_Rect currentViewport;
    int currentViewportRotation;
    SDL_bool viewportDirty;
    Float4X4 identity;
    int currentVertexBuffer;
    SDL_bool issueBatch;
} D3D12_RenderData;

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

    /**
        * A rectangle, with the origin at the upper left (double precision).
        */
    typedef struct SDL_DRect
    {
        double x;
        double y;
        double w;
        double h;
    } SDL_DRect;

    /* The SDL 2D rendering system */

    typedef struct SDL_RenderDriver SDL_RenderDriver;

    /* Rendering view state */
    typedef struct SDL_RenderViewState
    {
        int pixel_w;
        int pixel_h;
        SDL_Rect viewport;
        SDL_Rect clip_rect;
        SDL_bool clipping_enabled;
        SDL_FPoint scale;

    } SDL_RenderViewState;


    typedef enum
    {
        SDL_RENDERCMD_NO_OP,
        SDL_RENDERCMD_SETVIEWPORT,
        SDL_RENDERCMD_SETCLIPRECT,
        SDL_RENDERCMD_SETDRAWCOLOR,
        SDL_RENDERCMD_CLEAR,
        SDL_RENDERCMD_DRAW_POINTS,
        SDL_RENDERCMD_DRAW_LINES,
        SDL_RENDERCMD_FILL_RECTS,
        SDL_RENDERCMD_COPY,
        SDL_RENDERCMD_COPY_EX,
        SDL_RENDERCMD_GEOMETRY
    } SDL_RenderCommandType;

    typedef struct SDL_RenderCommand
    {
        SDL_RenderCommandType command;
        union
        {
            struct
            {
                size_t first;
                SDL_Rect rect;
            } viewport;
            struct
            {
                SDL_bool enabled;
                SDL_Rect rect;
            } cliprect;
            struct
            {
                size_t first;
                size_t count;
                Uint8 r, g, b, a;
                SDL_BlendMode blend;
                SDL_Texture* texture;
            } draw;
            struct
            {
                size_t first;
                Uint8 r, g, b, a;
            } color;
        } data;
        struct SDL_RenderCommand* next;
    } SDL_RenderCommand;

    typedef struct SDL_VertexSolid
    {
        SDL_FPoint position;
        SDL_Color color;
    } SDL_VertexSolid;

    typedef enum
    {
        SDL_RENDERLINEMETHOD_POINTS,
        SDL_RENDERLINEMETHOD_LINES,
        SDL_RENDERLINEMETHOD_GEOMETRY,
    } SDL_RenderLineMethod;

    typedef enum
    {
        SDL_LOGICAL_PRESENTATION_DISABLED,  /**< There is no logical size in effect */
        SDL_LOGICAL_PRESENTATION_STRETCH,   /**< The rendered content is stretched to the output resolution */
        SDL_LOGICAL_PRESENTATION_LETTERBOX, /**< The rendered content is fit to the largest dimension and the other dimension is letterboxed with black bars */
        SDL_LOGICAL_PRESENTATION_OVERSCAN,  /**< The rendered content is fit to the smallest dimension and the other dimension extends beyond the output bounds */
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE,  /**< The rendered content is scaled up by integer multiples to fit the output resolution */
    } SDL_RendererLogicalPresentation;

    /* Define the SDL renderer structure */
    struct SDL_Renderer
    {
        const void* magic;

        void (*WindowEvent) (SDL_Renderer* renderer, const SDL_WindowEvent* event);
        int (*GetOutputSize) (SDL_Renderer* renderer, int* w, int* h);
        SDL_bool(*SupportsBlendMode)(SDL_Renderer* renderer, SDL_BlendMode blendMode);
        int (*CreateTexture) (SDL_Renderer* renderer, SDL_Texture* texture);
        int (*QueueSetViewport) (SDL_Renderer* renderer, SDL_RenderCommand* cmd);
        int (*QueueSetDrawColor) (SDL_Renderer* renderer, SDL_RenderCommand* cmd);
        int (*QueueDrawPoints) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, const SDL_FPoint* points,
            int count);
        int (*QueueDrawLines) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, const SDL_FPoint* points,
            int count);
        int (*QueueFillRects) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, const SDL_FRect* rects,
            int count);
        int (*QueueCopy) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, SDL_Texture* texture,
            const SDL_Rect* srcrect, const SDL_FRect* dstrect);
        int (*QueueCopyEx) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, SDL_Texture* texture,
            const SDL_Rect* srcquad, const SDL_FRect* dstrect,
            const double angle, const SDL_FPoint* center, const SDL_RendererFlip flip, float scale_x, float scale_y);
        int (*QueueGeometry) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, SDL_Texture* texture,
            const float* xy, int xy_stride, const SDL_Color* color, int color_stride, const float* uv, int uv_stride,
            int num_vertices, const void* indices, int num_indices, int size_indices,
            float scale_x, float scale_y);

        int (*RunCommandQueue) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, void* vertices, size_t vertsize);
        int (*UpdateTexture) (SDL_Renderer* renderer, SDL_Texture* texture,
            const SDL_Rect* rect, const void* pixels,
            int pitch);
#if SDL_HAVE_YUV
        int (*UpdateTextureYUV) (SDL_Renderer* renderer, SDL_Texture* texture,
            const SDL_Rect* rect,
            const Uint8* Yplane, int Ypitch,
            const Uint8* Uplane, int Upitch,
            const Uint8* Vplane, int Vpitch);
        int (*UpdateTextureNV) (SDL_Renderer* renderer, SDL_Texture* texture,
            const SDL_Rect* rect,
            const Uint8* Yplane, int Ypitch,
            const Uint8* UVplane, int UVpitch);
#endif
        int (*LockTexture) (SDL_Renderer* renderer, SDL_Texture* texture,
            const SDL_Rect* rect, void** pixels, int* pitch);
        void (*UnlockTexture) (SDL_Renderer* renderer, SDL_Texture* texture);
        void (*SetTextureScaleMode) (SDL_Renderer* renderer, SDL_Texture* texture, SDL_ScaleMode scaleMode);
        int (*SetRenderTarget) (SDL_Renderer* renderer, SDL_Texture* texture);
        int (*RenderReadPixels) (SDL_Renderer* renderer, const SDL_Rect* rect,
            Uint32 format, void* pixels, int pitch);
        int (*RenderPresent) (SDL_Renderer* renderer);
        void (*DestroyTexture) (SDL_Renderer* renderer, SDL_Texture* texture);

        void (*DestroyRenderer) (SDL_Renderer* renderer);

        int (*SetVSync) (SDL_Renderer* renderer, int vsync);

        int (*GL_BindTexture) (SDL_Renderer* renderer, SDL_Texture* texture, float* texw, float* texh);
        int (*GL_UnbindTexture) (SDL_Renderer* renderer, SDL_Texture* texture);

        void* (*GetMetalLayer) (SDL_Renderer* renderer);
        void* (*GetMetalCommandEncoder) (SDL_Renderer* renderer);

        /* The current renderer info */
        SDL_RendererInfo info;

        /* The window associated with the renderer */
        SDL_Window* window;
        SDL_bool hidden;

        /* Whether we should simulate vsync */
        SDL_bool wanted_vsync;
        SDL_bool simulate_vsync;
        Uint32 simulate_vsync_interval;
        Uint32 last_present;

        /* The logical resolution for rendering */
        int logical_w;
        int logical_h;
        int logical_w_backup;
        int logical_h_backup;

        /* Whether or not to force the viewport to even integer intervals */
        SDL_bool integer_scale;

        /* The drawable area within the window */
        SDL_DRect viewport;
        SDL_DRect viewport_backup;

        /* The clip rectangle within the window */
        SDL_DRect clip_rect;
        SDL_DRect clip_rect_backup;

        /* Whether or not the clipping rectangle is used. */
        SDL_bool clipping_enabled;
        SDL_bool clipping_enabled_backup;

        /* The render output coordinate scale */
        SDL_FPoint scale;
        SDL_FPoint scale_backup;

        /* The pixel to point coordinate scale */
        SDL_FPoint dpi_scale;

        /* Whether or not to scale relative mouse motion */
        SDL_bool relative_scaling;

        /* The method of drawing lines */
        SDL_RenderLineMethod line_method;

        /* List of triangle indices to draw rects */
        int rect_index_order[6];

        /* Remainder from scaled relative motion */
        float xrel;
        float yrel;

        /* The list of textures */
        SDL_Texture* textures;
        SDL_Texture* target;
        SDL_mutex* target_mutex;

        SDL_Color color;                    /**< Color for drawing operations values */
        SDL_BlendMode blendMode;            /**< The drawing blend mode */

        SDL_bool always_batch;
        SDL_bool batching;
        SDL_RenderCommand* render_commands;
        SDL_RenderCommand* render_commands_tail;
        SDL_RenderCommand* render_commands_pool;
        Uint32 render_command_generation;
        Uint32 last_queued_color;
        SDL_DRect last_queued_viewport;
        SDL_DRect last_queued_cliprect;
        SDL_bool last_queued_cliprect_enabled;
        SDL_bool color_queued;
        SDL_bool viewport_queued;
        SDL_bool cliprect_queued;

        void* vertex_data;
        size_t vertex_data_used;
        size_t vertex_data_allocation;

        void* driverdata;
    };

    const int str_sz = sizeof(SDL_Renderer);

    /* Define the SDL render driver structure */
    struct SDL_RenderDriver
    {
        SDL_Renderer* (*CreateRenderer)(SDL_Window* window, Uint32 flags);

        /* Info about the renderer capabilities */
        SDL_RendererInfo info;
    };

    /* Per-texture data */
    typedef struct
    {
        ID3D11Texture2D* mainTexture;
        ID3D11ShaderResourceView* mainTextureResourceView;
        ID3D11RenderTargetView* mainTextureRenderTargetView;
        ID3D11Texture2D* stagingTexture;
        int lockedTexturePositionX;
        int lockedTexturePositionY;
        D3D11_FILTER scaleMode;
#if SDL_HAVE_YUV
        /* YV12 texture support */
        SDL_bool yuv;
        ID3D11Texture2D* mainTextureU;
        ID3D11ShaderResourceView* mainTextureResourceViewU;
        ID3D11Texture2D* mainTextureV;
        ID3D11ShaderResourceView* mainTextureResourceViewV;

        /* NV12 texture support */
        SDL_bool nv12;
        ID3D11Texture2D* mainTextureNV;
        ID3D11ShaderResourceView* mainTextureResourceViewNV;

        Uint8* pixels;
        int pitch;
        SDL_Rect locked_rect;
#endif
    } D3D11_TextureData;

    /* Blend mode data */
    typedef struct
    {
        SDL_BlendMode blendMode;
        ID3D11BlendState* blendState;
    } D3D11_BlendMode;

    /* Private renderer data */
    typedef struct
    {
        void* hDXGIMod;
        void* hD3D11Mod;
        IDXGIFactory2* dxgiFactory;
        IDXGIAdapter* dxgiAdapter;
        ID3D11Device1* d3dDevice;
        ID3D11DeviceContext1* d3dContext;
        IDXGISwapChain1* swapChain;
        DXGI_SWAP_EFFECT swapEffect;
        ID3D11RenderTargetView* mainRenderTargetView;
        ID3D11RenderTargetView* currentOffscreenRenderTargetView;
        ID3D11InputLayout* inputLayout;
        ID3D11Buffer* vertexBuffers[8];
        size_t vertexBufferSizes[8];
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShaders[NUM_SHADERS];
        int blendModesCount;
        D3D11_BlendMode* blendModes;
        ID3D11SamplerState* nearestPixelSampler;
        ID3D11SamplerState* linearSampler;
        D3D_FEATURE_LEVEL featureLevel;

        /* Rasterizers */
        ID3D11RasterizerState* mainRasterizer;
        ID3D11RasterizerState* clippedRasterizer;

        /* Vertex buffer constants */
        VertexShaderConstants vertexShaderConstantsData;
        ID3D11Buffer* vertexShaderConstants;

        /* Cached renderer properties */
        DXGI_MODE_ROTATION rotation;
        ID3D11RenderTargetView* currentRenderTargetView;
        ID3D11RasterizerState* currentRasterizerState;
        ID3D11BlendState* currentBlendState;
        ID3D11PixelShader* currentShader;
        ID3D11ShaderResourceView* currentShaderResource;
        ID3D11SamplerState* currentSampler;
        SDL_bool cliprectDirty;
        SDL_bool currentCliprectEnabled;
        SDL_Rect currentCliprect;
        SDL_Rect currentViewport;
        int currentViewportRotation;
        SDL_bool viewportDirty;
        Float4X4 identity;
        int currentVertexBuffer;
    } D3D11_RenderData;

#ifdef __cplusplus
}
#endif