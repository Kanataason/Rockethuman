#pragma once

#include"BaseSystem.h"
#include"Renderer.h"
#include "BaseScene.h"
#include "BaseGameObjectFactory.h"
#include "ModelManager.h"
#include "ModelLoader.h"
#include "MaterialManager.h"
#include "MaterialLoader.h"
#include "TextureManager.h"
#include "InputSystem.h"
#include "DebagUtility.h"
#include "AnimationLoader.h"
#include "SceneManager.h"
#include "UiRenderer.h"

#include "PhysicsSystem.h"
#include "Timer.h"
#include "TimeScale.h"
#include "SoundManager.h"

class GameManager
{
public :
    GameManager() :
        _materialManager(_materialLoader, _textureManager),
        _modelManager(_modelLoader, _animationLoader),
        _physicsSystem(&_debugDraw),
        _gameObjectFactory(_modelManager, _materialManager)
    {
    }
	void Initialize(HWND hwnd);
	void Update();
	void Draw();
	void Release();
private:
    // 各システムへのポインタをまとめてサービスロケータとして渡す
    ServiceLocator CreateServiceLocator();

    void InitializeFactory();
    void InitializeSystem(HWND hwnd);
private:
    float accumulator = 0.0f;

    std::unique_ptr<InputSystem> _input;
    BaseSystem _baseSystem;
    Renderer _renderer;
    Timer _timer;
    TimeScale _timeScale;   // スローモーションなどの、時間の倍率
    SoundManager _soundManager;   // 効果音とBGM(シーンをまたいで、1つだけ持つ)

    TextureManager _textureManager;

    AnimationLoader _animationLoader;
    MaterialLoader _materialLoader;
    MaterialManager _materialManager;

    ModelLoader _modelLoader;
    ModelManager _modelManager;

    DebugDraw _debugDraw;
    PhysicsSystem _physicsSystem;

    SceneManager _sceneManager;
    UiRenderer _uiRenderer;

    GameObjectFactory _gameObjectFactory;
};