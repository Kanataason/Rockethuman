#pragma once

#include <d3dcompiler.h>
#include <d3d11.h>

class InputSystem;
class PhysicsSystem;
class CollisionManager;
class DebugDraw;
class GameObjectFactory;
class UiRenderer;
class SceneManager;
class TimeScale;

/// <summary>
/// 他のコンポーネントでも必要となる物を入れる構造体
/// </summary>
struct ServiceLocator
{
    InputSystem* input = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    DebugDraw* debag = nullptr;
    GameObjectFactory* factory = nullptr;
    SceneManager* sceneManager = nullptr;

    TimeScale* timeScale = nullptr;
    UiRenderer* uiRenderer = nullptr;
    PhysicsSystem* physicsSystem = nullptr;
};