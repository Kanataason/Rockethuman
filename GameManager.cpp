#include <Windows.h>
#include <string>

#include "GameManager.h"

#include"BaseSystem.h"
#include"Renderer.h"
#include "BaseCamera.h"

#include "Transform.h"
#include "Mesh.h"
#include "ServiceLoacator.h"
// 60fps固定で物理演算を回すための固定タイムステップ
constexpr float fixedDeltaTime = 1.0f / 60.0f;
void GameManager::Initialize(HWND hwnd)
{
    _input = std::make_unique<InputSystem>(hwnd);

    InitializeSystem(hwnd);
    InitializeFactory();

    // 各システムへの参照をまとめてシーン側に渡す
    auto context = CreateServiceLocator();
    _debugDraw.Initialize(context);
    _sceneManager.Initialize(context,&_physicsSystem);
    _uiRenderer.Initialize(context.device, context.context,
        L"Asset/Text/UIFont.spritefont", (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
}
void GameManager::InitializeFactory()
{
    _textureManager.SetDeviceAndContext(_baseSystem.GetDevice(), _baseSystem.GetContext());
    _modelManager.SetDevice(_baseSystem.GetDevice());
    _materialManager.Initialize();
}
void GameManager::InitializeSystem(HWND hwnd)
{
    // 描画・入力まわりの基盤システムを初期化
    _baseSystem.Initialize(hwnd);
    _renderer.Initialize(_baseSystem.GetDevice(), _baseSystem.GetContext());

    _timer.Initialize();
}

void GameManager::Update()
{
    _baseSystem.ClearScreen();//画面の初期化

    float deltaTime = _timer.Tick();

    _input->Update();

    _sceneManager.Update(deltaTime);

    // 固定ステップで物理更新（フレームレート非依存にするため）
    accumulator += deltaTime;
    while (accumulator >= fixedDeltaTime)
    {
        _physicsSystem.FixedUpdate(fixedDeltaTime);
        accumulator -= fixedDeltaTime;
    }

    Draw();

    _baseSystem.Present();//描画
}
void GameManager::Draw()
{
    _sceneManager.Draw(&_renderer);
}

void GameManager::Release()
{
    _baseSystem.Release();

    _sceneManager.Release();

}

ServiceLocator GameManager::CreateServiceLocator() 
{
    ServiceLocator context = { };
    context.input = _input.get();
    context.context = _baseSystem.GetContext();
    context.device = _baseSystem.GetDevice();
    context.physicsSystem = &_physicsSystem;
    context.uiRenderer = &_uiRenderer;
    context.factory = &_gameObjectFactory;
    context.sceneManager = &_sceneManager;

    return context;
}