/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstException.h>
#include <Graphics/Renderer.h>
#include <Graphics/SpriteBatch.h>

using namespace Graphics;

SpriteBatch::SpriteBatch()
{
    RenderMode = SpriteBatchMode::Deferred;
    SortMode = SpriteBatchSortMode::BackToFront;
    m_DrawingMode = false;
    m_CurrentQueue = 0;
    m_Image = nullptr;
}

SpriteBatch::~SpriteBatch()
{
}

void SpriteBatch::Begin()
{
    if (m_DrawingMode) {
        throw Exceptions::EstException("SpriteBatch already on draw mode!");
    }

    m_Queue.clear();

    m_Image = nullptr;
    m_DrawingMode = true;
}

void SpriteBatch::Draw(Backends::SubmitInfo info)
{
    if (!m_Image) {
        m_Image = info.image;
    } else {
        if (m_Image != info.image) {
            throw Exceptions::EstException("SpriteBatch can't draw different images at the same time!");
        }
    }

    switch (RenderMode) {
        case SpriteBatchMode::Immediate:
        {
            auto renderer = Graphics::Renderer::Get()->GetBackend();
            renderer->Push(info);
            break;
        }

        case SpriteBatchMode::Deferred:
        {
            m_Queue[m_CurrentQueue++].push_back(info);
            break;
        }
    }
}

void SpriteBatch::Draw(std::vector<Backends::SubmitInfo> infos)
{
    if (!m_Image) {
        m_Image = infos[0].image;
    } else {
        if (m_Image != infos[0].image) {
            throw Exceptions::EstException("SpriteBatch can't draw different images at the same time!");
        }
    }

    switch (RenderMode) {
        case SpriteBatchMode::Immediate:
        {
            auto renderer = Graphics::Renderer::Get()->GetBackend();
            renderer->Push(infos);
            break;
        }

        case SpriteBatchMode::Deferred:
        {
            auto &vec = m_Queue[m_CurrentQueue++];
            for (int i = 0; i < infos.size(); i++) {
                vec.push_back(infos[i]);
            }
            break;
        }
    }
}

void SpriteBatch::End()
{
    if (!m_DrawingMode) {
        throw Exceptions::EstException("SpriteBatch not on draw mode!");
    }

    m_DrawingMode = false;

    if (m_Queue.size()) {
        std::vector<Backends::SubmitInfo> vec;

        switch (SortMode) {
            case SpriteBatchSortMode::BackToFront:
            {
                for (auto it = m_Queue.begin(); it != m_Queue.end(); it++) {
                    for (int i = 0; i < it->second.size(); i++) {
                        vec.push_back(it->second[i]);
                    }
                }
                break;
            }

            case SpriteBatchSortMode::FrontToBack:
            {
                for (auto it = m_Queue.rbegin(); it != m_Queue.rend(); it++) {
                    for (int i = 0; i < it->second.size(); i++) {
                        vec.push_back(it->second[i]);
                    }
                }
                break;
            }
        }

        auto renderer = Graphics::Renderer::Get()->GetBackend();
        renderer->Push(vec);
    }
}