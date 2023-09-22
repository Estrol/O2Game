#include "ReloadScene.h"
#include <SceneManager.h>

ReloadScene::ReloadScene() {
}

void ReloadScene::Update(double delta) {
    m_time += delta;

    if (m_time >= 0.25) {
        SceneManager::ChangeScene(SceneManager::GetInstance()->GetLastSceneIndex());
    }
}

bool ReloadScene::Attach() {
    m_time = 0;
    return true;
}

bool ReloadScene::Detach() {
    return true;
}
