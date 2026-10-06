#include "Title.h"

#include <array>

#include "InputSystem.h"
#include "GameObject.h"
#include "CameraController.h"
#include "UiRenderer.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "CharacterRoster.h"

using namespace UiInfo;

namespace
{
	// ステージ選択の並び。panelList の StageSelect の items と、同じ順番にすること
	constexpr std::array<SceneName, 2> stageScenes = { SceneName::Stage1, SceneName::Stage2 };
	constexpr int stageCount = static_cast<int>(stageScenes.size());

	// キャラクター選択の、名前の行(パネルの中に、上から並べる)
	constexpr float characterRowLeft = 490.0f;
	constexpr float characterRowTop = 250.0f;
	constexpr float characterRowWidth = 300.0f;
	constexpr float characterRowHeight = 60.0f;
	constexpr float characterRowGap = 10.0f;
	constexpr float characterRowTextScale = 0.5f;
}

void Title::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	_characters = locator.characters;
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

	case PanelType::CharacterSelect:
		UpdateCharacterSelect();
		break;
	default:
		// Enterで閉じる
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

	if (_input->GetKeyDown(VK_SPACE))
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
	if (_input->GetKeyDown(VK_ESCAPE) || _input->GetKeyDown(VK_RETURN))
		_currentPanel = PanelType::None;
}

void Title::UpdateCharacterSelect()
{
	// 選ぶと、すぐに「選んでいる人」が変わる(決定は、パネルを閉じるだけ)。ステージを始めるときに、この人が出る
	if (_characters)
	{
		if (_input->GetKeyDown('W') || _input->GetKeyDown(VK_UP))
			_characters->SelectPrevious();

		if (_input->GetKeyDown('S') || _input->GetKeyDown(VK_DOWN))
			_characters->SelectNext();
	}

	if (_input->GetKeyDown(VK_SPACE) || _input->GetKeyDown(VK_RETURN) || _input->GetKeyDown(VK_ESCAPE))
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
		// 「キャラクターセレクト」: キャラクター選択を開く
		_currentPanel = PanelType::CharacterSelect;
		break;
	case TextType::End:
		PostQuitMessage(0);
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

	case PanelType::CharacterSelect:
		DrawCharacterSelect();
		break;

	default:
		DrawPanel(_currentPanel, -1);
		break;
	}

	_uiRenderer->DrawString(L"ロケットにんげん", 380.0f, 40.0f, 1.0f,
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

void Title::DrawCharacterSelect()
{
	DrawPanel(PanelType::CharacterSelect, -1);
	DrawText(TextType::CharacterSelectTitle);

	// キャラクターの名前を、上から並べる(いま選んでいる人は、黄色)
	if (_characters && _characters->GetCount() > 0)
	{
		const auto& list = _characters->GetList();
		for (int i = 0; i < _characters->GetCount(); i++)
		{
			const float top = characterRowTop + (characterRowHeight + characterRowGap) * static_cast<float>(i);
			const bool selected = (i == _characters->GetSelectedIndex());

			_uiRenderer->DrawFrame(characterRowLeft, top, characterRowWidth, characterRowHeight, 2.0f,
				selected ? XMVectorSet(1, 1, 0, 1) : XMVectorSet(1, 1, 1, 1));
			_uiRenderer->DrawStringCentered(list[static_cast<size_t>(i)].name,
				characterRowLeft + characterRowWidth * 0.5f, top + characterRowHeight * 0.5f,
				characterRowTextScale,
				selected ? XMVectorSet(1, 1, 0, 1) : XMVectorSet(1, 1, 1, 1),
				XMVectorSet(0, 0, 0, 1));
		}
	}
	else
	{
		// 一覧が空(Characters.jsonがない)ときは、ステージの標準のキャラクターが出る
		_uiRenderer->DrawStringCentered(L"キャラクターが ありません", 640.0f, 330.0f, 0.45f,
			XMVectorSet(1, 1, 1, 1), XMVectorSet(0, 0, 0, 1));
	}

	DrawText(TextType::CharacterSelectHint);
}
