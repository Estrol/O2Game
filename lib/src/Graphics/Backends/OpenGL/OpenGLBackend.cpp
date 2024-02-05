/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#define NOMINMAX
#include "OpenGLBackend.h"
#include <iostream>

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

#include "../../Shaders/SPV/image.spv.h"
#include "../../Shaders/SPV/position.spv.h"
#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>
#include <algorithm>

#include "../../ImguiBackends/imgui_impl_opengl3.h"
#include "../../ImguiBackends/imgui_impl_sdl2.h"

#include <spirv_cross/spirv_glsl.hpp>

using namespace Graphics::Backends;

struct PushConstant
{
    glm::vec4 ui_radius;
    glm::vec2 ui_size;

    glm::vec2 scale;
    glm::vec2 translate;
};

uint32_t glBlendOperatioId;

std::string compileSPRIV(const uint32_t *data, size_t size)
{
    spirv_cross::CompilerGLSL compiler(data, size);

    spirv_cross::CompilerGLSL::Options options;
    options.version = 430;
    options.emit_push_constant_as_uniform_buffer = true;
    compiler.set_common_options(options);

    return compiler.compile();
}

void OpenGL::Init()
{
    auto window = Graphics::NativeWindow::Get();
    if (SDL_GL_LoadLibrary(NULL) != 0) {
        throw Exceptions::EstException("Failed to load OpenGL library");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // create context
    SDL_GLContext context = SDL_GL_CreateContext(window->GetWindow());

    if (context == nullptr) {
        throw Exceptions::EstException("Failed to create OpenGL context");
    }

    if (SDL_GL_MakeCurrent(window->GetWindow(), context) != 0) {
        throw Exceptions::EstException("Failed to make OpenGL context current");
    }

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
    if (version == 0) {
        throw Exceptions::EstException("Failed to load OpenGL functions");
    }

    Data.ctx = context;

    constexpr uint32_t MAX_VERTEX_OBJECTS = 50000;
    constexpr uint32_t MAX_VERTEX_BUFFER_SIZE = sizeof(Vertex) * MAX_VERTEX_OBJECTS;
    constexpr uint32_t MAX_INDEX_BUFFER_SIZE = sizeof(uint32_t) * MAX_VERTEX_OBJECTS;

    glGenBuffers(1, &Data.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Data.vertexBuffer);

    glGenBuffers(1, &Data.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Data.indexBuffer);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    // set viewport
    glViewport(0, 0, window->GetWindowSize().Width, -window->GetWindowSize().Height);

    Data.maxVertexBufferSize = MAX_VERTEX_BUFFER_SIZE;
    Data.maxIndexBufferSize = MAX_INDEX_BUFFER_SIZE;

    // float[2] scale, translation uniform buffer
    glGenBuffers(1, &Data.constantBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, Data.constantBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PushConstant), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, Data.constantBuffer);

    CreateShader();
    CreateDefaultBlend();
    ImGui_Init();

    int error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cout << "[OpenGL] [Error] " << error << std::endl;
    }
}

void OpenGL::CreateShader()
{
    std::vector<std::pair<ShaderFragmentType, std::pair<const uint32_t *, size_t>>> shaders = {
        { ShaderFragmentType::Solid, { __glsl_image, sizeof(__glsl_image) / sizeof(__glsl_image[0]) } },
        { ShaderFragmentType::Image, { __glsl_image, sizeof(__glsl_image) / sizeof(__glsl_image[0]) } }
    };

    for (auto &[type, shader] : shaders) {
        auto          vertex = compileSPRIV(__glsl_position, sizeof(__glsl_position) / sizeof(__glsl_position[0]));
        const GLchar *sourcevertex = (const GLchar *)vertex.c_str();

        GLuint shaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shaderId, 1, &sourcevertex, nullptr);
        glCompileShader(shaderId);

        GLint errcode = GL_TRUE;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &errcode);

        if (errcode != GL_TRUE) {
            throw Exceptions::EstException("Failed to compile vertex shader");
        }

        auto          fragment = compileSPRIV(shader.first, shader.second);
        const GLchar *sourcefragment = (const GLchar *)fragment.c_str();

        GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentId, 1, &sourcefragment, nullptr);
        glCompileShader(fragmentId);

        glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &errcode);

        if (errcode != GL_TRUE) {
            throw Exceptions::EstException("Failed to compile fragment shader");
        }

        GLuint programId = glCreateProgram();
        glAttachShader(programId, shaderId);
        glAttachShader(programId, fragmentId);
        glLinkProgram(programId);

        glGetProgramiv(programId, GL_LINK_STATUS, &errcode);

        if (errcode != GL_TRUE) {
            GLint length;
            glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> log(length);
            glGetProgramInfoLog(programId, length, &length, &log[0]);
            std::cerr << "Linker error: " << &log[0] << std::endl;

            throw Exceptions::EstException("Failed to link shader program");
        }

        GLint uniformLocation = glGetUniformLocation(programId, "sTexture");

        Data.shaders[type] = { shaderId, fragmentId, programId, uniformLocation };
    }
}

void OpenGL::CreateDefaultBlend()
{
    // NONE, no blending
    // BLEND, dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA)), dstA = srcA + (dstA * (1-srcA))
    // ADD, dstRGB = (srcRGB * srcA) + dstRGB, dstA = dstA
    // MOD, dstRGB = srcRGB * dstRGB, dstA = dstA
    // MUL dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA)), dstA = (srcA * dstA) + (dstA * (1-srcA))

    TextureBlendInfo blendNone = {
        true,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendBlend = {
        true,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendAdd = {
        true,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendMod = {
        true,
        BlendFactor::BLEND_FACTOR_DST_COLOR,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendMul = {
        true,
        BlendFactor::BLEND_FACTOR_SRC_COLOR,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD
    };

    CreateBlendState(blendNone);
    CreateBlendState(blendBlend);
    CreateBlendState(blendAdd);
    CreateBlendState(blendMod);
    CreateBlendState(blendMul);
}

void OpenGL::Shutdown()
{
    for (GLuint texture : textures) {
        glDeleteTextures(1, &texture);
    }

    ImGui_DeInit();

    textures.clear();

    // free the buffer
    glDeleteBuffers(1, &Data.vertexBuffer);
    glDeleteBuffers(1, &Data.indexBuffer);
    glDeleteBuffers(1, &Data.constantBuffer);

    // delete shader program
    for (auto &[type, shader] : Data.shaders) {
        glDeleteProgram(shader.program);
        glDeleteShader(shader.vert);
        glDeleteShader(shader.frag);
    }

    SDL_GL_DeleteContext(Data.ctx);

    gladLoaderUnloadGL();
}

void OpenGL::ReInit()
{
}

bool OpenGL::NeedReinit()
{
    return false;
}

bool OpenGL::BeginFrame()
{
    auto rect = Graphics::NativeWindow::Get()->GetWindowSize();
    if (!rect.Width || !rect.Height) {
        return false;
    }

    glEnable(GL_SCISSOR_TEST);
    glViewport(0, 0, rect.Width, -rect.Height);
    glScissor(0, 0, rect.Width, rect.Height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
}

void OpenGL::EndFrame()
{
    FlushQueue();

    if (m_HasImgui) {
        m_HasImgui = false;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    SDL_GL_SwapWindow((SDL_Window *)Graphics::NativeWindow::Get()->GetWindow());
    FlushScreenshotQueue();
}

void OpenGL::FlushScreenshotQueue()
{
    if (screenshotQueues.size() == 0) {
        return;
    }

    auto                       rect = Graphics::NativeWindow::Get()->GetWindowSize();
    std::vector<unsigned char> pixels(rect.Width * rect.Height * 4);

    glReadPixels(0, 0, rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    for (int y = 0; y < rect.Height / 2; y++) {
        for (int x = 0; x < rect.Width; x++) {
            for (int c = 0; c < 4; c++) {
                std::swap(pixels[(y * rect.Width + x) * 4 + c], pixels[((rect.Height - 1 - y) * rect.Width + x) * 4 + c]);
            }
        }
    }

    for (auto &it : screenshotQueues) {
        it(pixels);
    }

    screenshotQueues.clear();
}

void OpenGL::Push(SubmitInfo &info)
{
    uint32_t maxSize = drawData.vertexSize + (uint32_t)info.vertices.size();

    if (maxSize >= drawData.vertex.size()) {
        drawData.vertex.resize(maxSize);
    }

    uint32_t inMaxSize = drawData.indiceSize + (uint32_t)info.indices.size();

    if (inMaxSize >= drawData.indices.size()) {
        drawData.indices.resize(inMaxSize);
    }

    for (auto &it : info.vertices) {
        it.pos.x = std::round(it.pos.x);
        it.pos.y = std::round(it.pos.y);

        drawData.vertex[drawData.vertexSize++] = it;
    }

    for (auto &it : info.indices) {
        drawData.indices[drawData.indiceSize++] = it + drawData.currentIndexCount;
    }

    drawData.currentIndexCount += (uint32_t)info.vertices.size();

    OpenGLDrawItem item = {};
    item.blend = info.alphablend;
    item.count = (uint32_t)info.indices.size();
    item.instanceCount = 1;
    item.type = info.fragmentType;
    item.uiRadius = info.uiRadius;
    item.image = info.image;
    item.uiSize = info.uiSize;
    item.clipRect = info.clipRect;

    submitInfos.push_back(item);
}

void OpenGL::Push(std::vector<SubmitInfo> &infos)
{
    if (infos.size() == 0) {
        return;
    }

    uint32_t maxSize = drawData.vertexSize + (uint32_t)(infos[0].vertices.size() * infos.size());

    if (maxSize >= drawData.vertex.size()) {
        drawData.vertex.resize(maxSize);
    }

    uint32_t inMaxSize = drawData.indiceSize + (uint32_t)(infos[0].indices.size() * infos.size());

    if (inMaxSize >= drawData.indices.size()) {
        drawData.indices.resize(inMaxSize);
    }

    for (auto &info : infos) {
        for (auto &it : info.vertices) {
            it.pos.x = std::round(it.pos.x);
            it.pos.y = std::round(it.pos.y);

            drawData.vertex[drawData.vertexSize++] = it;
        }

        for (auto &it : info.indices) {
            drawData.indices[drawData.indiceSize++] = it + drawData.currentIndexCount;
        }

        drawData.currentIndexCount += (uint32_t)info.vertices.size();
    }

    OpenGLDrawItem item = {};
    item.blend = infos[0].alphablend;
    item.count = (uint32_t)infos[0].indices.size();
    item.instanceCount = (uint32_t)infos.size();
    item.type = infos[0].fragmentType;
    item.uiRadius = infos[0].uiRadius;
    item.image = infos[0].image;
    item.uiSize = infos[0].uiSize;
    item.clipRect = infos[0].clipRect;

    submitInfos.push_back(item);
}

GLenum mapBlendFactor(BlendFactor factor)
{
    switch (factor) {
        case BlendFactor::BLEND_FACTOR_ZERO:
            return GL_ZERO;
        case BlendFactor::BLEND_FACTOR_ONE:
            return GL_ONE;
        case BlendFactor::BLEND_FACTOR_SRC_COLOR:
            return GL_SRC_COLOR;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
            return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::BLEND_FACTOR_DST_COLOR:
            return GL_DST_COLOR;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_DST_COLOR:
            return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::BLEND_FACTOR_SRC_ALPHA:
            return GL_SRC_ALPHA;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::BLEND_FACTOR_DST_ALPHA:
            return GL_DST_ALPHA;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::BLEND_FACTOR_CONSTANT_COLOR:
            return GL_CONSTANT_COLOR;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
            return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::BLEND_FACTOR_CONSTANT_ALPHA:
            return GL_CONSTANT_ALPHA;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
            return GL_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::BLEND_FACTOR_SRC_ALPHA_SATURATE:
            return GL_SRC_ALPHA_SATURATE;
        case BlendFactor::BLEND_FACTOR_SRC1_COLOR:
            return GL_SRC1_COLOR;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
            return GL_ONE_MINUS_SRC1_COLOR;
        case BlendFactor::BLEND_FACTOR_SRC1_ALPHA:
            return GL_SRC1_ALPHA;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
            return GL_ONE_MINUS_SRC1_ALPHA;
        default:
            return GL_ZERO; // default case for BLEND_FACTOR_MAX_ENUM and any other unexpected value
    }
}

GLenum mapBlendOp(BlendOp op)
{
    switch (op) {
        case BlendOp::BLEND_OP_ADD:
            return GL_FUNC_ADD;
        case BlendOp::BLEND_OP_SUBTRACT:
            return GL_FUNC_SUBTRACT;
        case BlendOp::BLEND_OP_REVERSE_SUBTRACT:
            return GL_FUNC_REVERSE_SUBTRACT;
        case BlendOp::BLEND_OP_MIN:
            return GL_MIN;
        case BlendOp::BLEND_OP_MAX:
            return GL_MAX;
        default:
            return GL_FUNC_ADD; // default case for BLEND_OP_MAX_ENUM and any other unexpected value
    }
}

void setBlendInfo(const TextureBlendInfo &blendInfo)
{
    if (blendInfo.Enable) {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(
            mapBlendFactor(blendInfo.SrcColor),
            mapBlendFactor(blendInfo.DstColor),
            mapBlendFactor(blendInfo.SrcAlpha),
            mapBlendFactor(blendInfo.DstAlpha));

        glBlendEquationSeparate(
            mapBlendOp(blendInfo.ColorOp),
            mapBlendOp(blendInfo.AlphaOp));
    } else {
        glDisable(GL_BLEND);
    }
}

void OpenGL::FlushQueue()
{
    if (submitInfos.size() == 0) {
        return;
    }

    GLuint vertex_size = sizeof(drawData.vertex[0]) + (drawData.vertexSize + 1);
    GLuint indices_size = sizeof(drawData.indices[0]) + (drawData.indiceSize + 1);

    glBindBuffer(GL_ARRAY_BUFFER, Data.vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Data.indexBuffer);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_size, nullptr);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_size, nullptr);

    auto rect = Graphics::NativeWindow::Get()->GetWindowSize();

    GLuint vertex_offset = 0;
    GLuint indices_offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, Data.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * sizeof(Vertex), drawData.vertex.data(), GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Data.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size * sizeof(uint16_t), drawData.indices.data(), GL_STREAM_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)MY_OFFSETOF(Vertex, pos));

    // Texture coordinates attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)MY_OFFSETOF(Vertex, texCoord));

    // Color attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)MY_OFFSETOF(Vertex, color));

    PushConstant pc = {};
    pc.scale = glm::vec2(2.0f / rect.Width, -2.0f / rect.Height);
    pc.translate = glm::vec2(-1.0f, 1.0f);
    for (auto &info : submitInfos) {
        GLuint imageId = static_cast<GLuint>(reinterpret_cast<intptr_t>(info.image));

        pc.ui_radius = info.uiRadius;
        pc.ui_size = info.uiSize;

        glBindBuffer(GL_UNIFORM_BUFFER, Data.constantBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PushConstant), &pc);

        auto &shaderData = Data.shaders[info.type];

        glUseProgram(shaderData.program);

        GLint textureLocation = shaderData.location;
        if (textureLocation != -1 && imageId != -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, imageId);
            glUniform1d(textureLocation, 0);
        }

        auto &blend = blendStates[info.blend];
        setBlendInfo(blend);

        GLuint firstIndex = indices_offset;
        GLuint indexCount = (GLuint)info.count;

        GLint x = (GLint)info.clipRect.X;
        GLint y = 0;

        // Flip the Y axis
        GLint YOrigin = info.clipRect.Height;
        GLint HalfHeight = rect.Height / 2;
        GLint D = YOrigin - HalfHeight;
        GLint Y = YOrigin - (2 * D);

        y = Y;

        GLsizei w = (GLsizei)info.clipRect.Width;
        GLsizei h = (GLsizei)info.clipRect.Height;

        assert(x >= 0);
        assert(y >= 0);
        assert(w >= 0);
        assert(h >= 0);

        glScissor(x, y, w, h);

        glDrawElements(
            GL_TRIANGLES,
            indexCount * info.instanceCount,
            GL_UNSIGNED_SHORT,
            (void *)(firstIndex * sizeof(uint16_t)));

        vertex_offset += indexCount * info.instanceCount;
        indices_offset += indexCount * info.instanceCount;
    }

    submitInfos.clear();
    drawData.Reset();

    int error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cout << "[OpenGL] [Error] " << error << std::endl;
    }
}

GLuint OpenGL::CreateTexture()
{
    GLuint texture;
    glGenTextures(1, &texture);

    textures.push_back(texture);
    return texture;
}

void OpenGL::DestroyTexture(GLuint texture)
{
    auto it = std::find(textures.begin(), textures.end(), texture);
    if (it != textures.end()) {
        textures.erase(it);
    }

    glDeleteTextures(1, &texture);
}

void OpenGL::SetClearColor(glm::vec4 color)
{
}

void OpenGL::SetClearDepth(float depth)
{
}

void OpenGL::SetClearStencil(uint32_t stencil)
{
}

BlendHandle OpenGL::CreateBlendState(TextureBlendInfo blendInfo)
{
    blendStates[glBlendOperatioId] = blendInfo;
    return glBlendOperatioId++;
}

void OpenGL::CaptureFrame(std::function<void(std::vector<unsigned char>)> imageData)
{
    screenshotQueues.push_back(imageData);
}

void OpenGL::ImGui_Init()
{
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = NULL;

    auto window = Graphics::NativeWindow::Get();
    ImGui_ImplSDL2_InitForOpenGL(window->GetWindow(), Data.ctx);
    window->AddSDLCallback([=](SDL_Event &ev) {
        ImGui_ImplSDL2_ProcessEvent(&ev);
    });

    ImGui_ImplOpenGL3_Init();
}

void OpenGL::ImGui_DeInit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();
}

void OpenGL::ImGui_NewFrame()
{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    m_HasImgui = true;
}

void OpenGL::ImGui_EndFrame()
{
    ImGui::EndFrame();
    ImGui::Render();
}

void OpenGL::SetVSync(bool enabled)
{
    SDL_GL_SetSwapInterval((int)enabled);
}