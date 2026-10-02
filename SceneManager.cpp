#include <unordered_map>
#include "SceneManager.h"

#include "Title.h"
#include "GameScene.h"
#include "Result.h"


//シーンの作り方をテーブルで登録
namespace
{
	const std::unordered_map<SceneName,
		std::function<std::unique_ptr<BaseScene>()>> MakeSceneFunctionTable =
	{
		{SceneName::Title,[]{return std::make_unique<Title>();}},
		{SceneName::GameScene,[] {return std::make_unique<GameScene>();}},
		{SceneName::Result,[] {return std::make_unique<Result>();}}
	};
}

void SceneManager::Initialize(ServiceLocator& locator,PhysicsSystem* physicsSystem)
{
	_locator = locator;
	EntryScene();
}
void SceneManager::Update(float deltaTime)
{
	if (_nextScene != SceneName::None)
		RequestScene();

	_currentScene->Update(deltaTime);
}
void SceneManager::Draw(Renderer* renderer)
{
	_currentScene->Draw(*renderer);
	_currentScene->DrawUi();
}

void SceneManager::Release()
{
	_currentScene->Release();
}

void SceneManager::ChangeScene(SceneName name)
{
	_nextScene = name;
}
void SceneManager::RequestScene()
{
	const auto& data = _sceneList.at(_nextScene);
	_nextScene = SceneName::None;

	//消す前にReleaseを呼ぶ(RigidBody/Colliderの登録解除は、デストラクタが自動的に行う)
	if (_currentScene)
		_currentScene->Release();

	_currentScene.reset();

	_currentScene = data.scene();
	_sceneLoader.LoadSceneGameObject(_currentScene.get(), data.path, _jsonLoader, *_locator.factory);
    _currentScene->Initialize(_locator);
}

void SceneManager::EntryScene()
{
	const std::string scenesPath = "Asset/Data/Scene.json";

	const auto& root = _jsonLoader.LoadJsonData(scenesPath);

	for (auto& scene : root.at("Scenes"))
	{
		auto data = _jsonLoader.LoadSceneData(scene);
		
		const auto& it = MakeSceneFunctionTable.find(data.sceneName);
		if (it == MakeSceneFunctionTable.end())return;

		data.scene = it->second;	
		_sceneList[data.sceneName] = data;
	}
	ChangeScene(SceneName::Title);
}