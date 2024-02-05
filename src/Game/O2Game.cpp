/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "O2Game.h"
#include <Imgui/imgui.h>
#include <Screens/ScreenManager.h>

#include "../Scenes/Gameplay.h"
#include "../Scenes/Loading.h"

#include "../Scenes/SceneList.h"

#include "Env.h"
#include "MsgBoxEx.h"

O2Game::O2Game()
{
}

O2Game::~O2Game()
{
}

void O2Game::Run(int argv, char **argc)
{
    RunInfo runInfo = {};
#if defined(_DEBUG)
    runInfo.title = "O2Game (Debug)";
#else
    runInfo.title = "O2Game";
#endif

    runInfo.resolution = { 1920, 1080 };
    runInfo.buffer_resolution = { 800, 600 };
    runInfo.graphics = Graphics::API::Vulkan;
    runInfo.threadMode = ThreadMode::Multi;
    runInfo.renderFrameRate = 1000.0f;
    runInfo.renderVsync = true;
    runInfo.inputFrameRate = 1000.0f;
    runInfo.fixedFrameRate = 65.0f;

    Graphics::TextureSamplerInfo sampler = {};
    sampler.FilterMag = Graphics::TextureFilter::Nearest;
    sampler.FilterMin = Graphics::TextureFilter::Nearest;
    sampler.AddressModeU = Graphics::TextureAddressMode::ClampEdge;
    sampler.AddressModeV = Graphics::TextureAddressMode::ClampEdge;
    sampler.AddressModeW = Graphics::TextureAddressMode::ClampEdge;
    sampler.MipLodBias = 0.0f;
    sampler.AnisotropyEnable = false;
    sampler.MaxAnisotropy = 1.0f;
    sampler.CompareEnable = false;
    sampler.CompareOp = Graphics::TextureCompareOP::COMPARE_OP_ALWAYS;
    sampler.MinLod = 0.0f;
    sampler.MaxLod = Graphics::kMaxLOD;

    runInfo.samplerInfo = sampler;

    Game::Run(runInfo);
}

void O2Game::OnLoad()
{
    using namespace Graphics::Backends;
    Game::OnLoad();

    auto renderer = Graphics::Renderer::Get();

    TextureBlendInfo blendMul = {
        true,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo nonBlendMul = {
        true,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD
    };

    Env::SetInt("BlendNonAlpha", renderer->CreateBlendState(nonBlendMul));
    Env::SetInt("BlendAlpha", renderer->CreateBlendState(blendMul));

    auto scenemanager = Screens::Manager::Get();

    scenemanager->AddScreen(SceneList::LOADING, new Loading());
    scenemanager->AddScreen(SceneList::GAMEPLAY, new Gameplay());
    scenemanager->SetScreen(SceneList::LOADING);
}

void O2Game::OnUnload()
{
    Game::OnUnload();
}

void O2Game::OnInput(double delta)
{
    Game::OnInput(delta);
}

void O2Game::OnUpdate(double delta)
{
    Game::OnUpdate(delta);
}

void O2Game::OnDraw(double delta)
{
    Game::OnDraw(delta);
    MsgBox::Draw(delta);
}