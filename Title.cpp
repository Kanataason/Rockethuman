#include "Title.h"

#include <array>

#include "InputSystem.h"
#include "GameObject.h"
#include "CameraController.h"
#include "UiRenderer.h"
#include "SceneManager.h"
#include "SoundManager.h"

using namespace UiInfo;

namespace
{
	// ステージ選択の並び。panelList の StageSelect の items と、同じ順番にすること
	constexpr std::array<SceneName, 2> stageScenes = { SceneName::Stage1, SceneName::Stage2 };
	constexpr int stageCount = static_cast<int>(stageScenes.size());
}

void Title::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	BaseScene::Initialize(locator);

	if (locator.sound)
		locator.sound->PlayBgm("title");

	// W/Sはメニュー選択に使うので、タイトルのカメラは動かさない
	for (const auto& object : _objects)
	{
		if (auto cameraController = object->GetComponent<CameraController>())
			cameraController->SetMoveEnabled(false);
	}
}

void Title::Update(float deltaTime)
{
	switch (_currentPanel)
	{
	case PanelType::None:
		UpdateMenu();
		break;

	case PanelType::StageSelect:
		UpdateStageSelect();
		break;

	default:
		// 設定パネルなど: Enterで閉じる
		if (_input->GetKeyDown(VK_RETURN) || _input->GetKeyDown(VK_ESCAPE))
			_currentPanel = PanelType::None;
		break;
	}

	BaseScene::Update(deltaTime);
}

void Title::UpdateMenu()
{
	if (_input->GetKeyDown('W') || _input->GetKeyDown(VK_UP))
		_selectedIndex = (_selectedIndex + MenuCount - 1) % MenuCount;

	if (_input->GetKeyDown('S') || _input->GetKeyDown(VK_DOWN))
		_selectedIndex = (_selectedIndex + 1) % MenuCount;

	if (_input->GetKeyDown(VK_SPACE) || _input->GetKeyDown(VK_RETURN))
		ConfirmSelected();
}

void Title::UpdateStageSelect()
{
	if (_input->GetKeyDown('W') || _input->GetKeyDown(VK_UP))
		_stageIndex = (_stageIndex + stageCount - 1) % stageCount;

	if (_input->GetKeyDown('S') || _input->GetKeyDown(VK_DOWN))
		_stageIndex = (_stageIndex + 1) % stageCount;

	// 決定: 選んだステージを、遊び始める(シーンの切り替えは予約。次のフレームの頭に入れ替わる)
	if (_input->GetKeyDown(VK_SPACE))
	{
		_sceneManager->ChangeScene(stageScenes[_stageIndex]);
		return;
	}

	// もどる: メニューに戻る
	if (_input->GetKeyDown(VK_RETURN) || _input->GetKeyDown(VK_ESCAPE))
		_currentPanel = PanelType::None;
}

void Title::ConfirmSelected()
{
	switch (MenuEntries[_selectedIndex].text)
	{
	case TextType::Start:
		// 「セレクトステージ」: ステージ選択を開く(前に選んだステージを、覚えたまま)
		_currentPanel = PanelType::StageSelect;
		break;
	case TextType::Setting:
		_currentPanel = PanelType::Setting;
		break;
	default:
		break;
	}
}

void Title::DrawUi()
{
	_uiRenderer->Begin();

	switch (_currentPanel)
	{
	case PanelType::None:
		for (int i = 0; i < MenuCount; i++)
			DrawMenuItem(MenuEntries[i], i == _selectedIndex);
		break;

	case PanelType::StageSelect:
		DrawStageSelect();
		break;

	default:
		DrawPanel(_currentPanel, -1);
		break;
	}

	_uiRenderer->DrawString(L"ロケットにんげん", 380.0f, 100.0f, 1.0f,
		DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
	_uiRenderer->End();
}

void Title::DrawText(TextType type)
{
	auto it = textList.find(type);
	if (it == textList.end()) return;
	const auto& textData = it->second;

	_uiRenderer->DrawStringCentered(textData.text, textData.centerX, textData.centerY, textData.scale,
		XMLoadFloat4(&textData.mainColor), XMLoadFloat4(&textData.outlineColor));
}

void Title::DrawMenuItem(MenuEntry menu, bool selected)
{
	auto buttonIterator = buttonList.find(menu.button);
	auto textIterator = textList.find(menu.text);
	if (buttonIterator == buttonList.end() || textIterator == textList.end()) return;

	const auto& buttonData = buttonIterator->second;
	const auto& textData = textIterator->second;

	_uiRenderer->DrawFrame(buttonData.left, buttonData.top, buttonData.width, buttonData.height,
		buttonData.thickness,
		selected ? XMVectorSet(1, 1, 0, 1) : XMLoadFloat4(&buttonData.frameColor));

	_uiRenderer->DrawStringCentered(textData.text, textData.centerX, textData.centerY, textData.scale,
		selected ? XMVectorSet(1, 1, 0, 1) : XMLoadFloat4(&textData.mainColor),
		XMLoadFloat4(&textData.outlineColor));
}

void Title::DrawPanel(PanelType typeIndex, int selectedIndex)
{
	auto it = panelList.find(typeIndex);
	if (it == panelList.end()) return;
	const PanelData& panel = it->second;

	_uiRenderer->DrawRect(panel.frame.left, panel.frame.top, panel.frame.width, panel.frame.height,
		XMLoadFloat4(&panel.fillColor));
	_uiRenderer->DrawFrame(panel.frame.left, panel.frame.top, panel.frame.width, panel.frame.height,
		panel.frame.thickness, XMLoadFloat4(&panel.frame.frameColor));

	for (int i = 0; i < static_cast<int>(panel.items.size()); i++)
		DrawMenuItem(panel.items[i], i == selectedIndex);
}

void Title::DrawStageSelect()
{
	DrawPanel(PanelType::StageSelect, _stageIndex);
	DrawText(TextType::StageSelectTitle);
	DrawText(TextType::StageSelectHint);
}
