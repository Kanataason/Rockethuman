#pragma once
#include "BaseScene.h"
#include "UiRenderer.h"
#include "UiLayout.h"
#include "Event.h"

class InputSystem;
class CharacterRoster;

class Title : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	// 画面の状態ごとの入力(メニュー / ステージ選択 / 設定パネル)
	void UpdateMenu();
	void UpdateStageSelect();
	void UpdateCharacterSelect();

	void ConfirmSelected();
	void DrawMenuItem(MenuEntry menu, bool selected);
	// selectedIndex: パネルの中で、選んでいる行(選ぶ物がないパネルは -1)
	void DrawPanel(PanelType typeIndex, int selectedIndex);
	void DrawStageSelect();
	void DrawCharacterSelect();
	void DrawText(TextType type);
private:
	int _selectedIndex = 0;     // メニュー(セレクトステージ / オプション / エンド)で選んでいる行
	int _stageIndex = 0;        // ステージ選択で選んでいる行

	PanelType _currentPanel = PanelType::None;

	InputSystem* _input = nullptr;
	CharacterRoster* _characters = nullptr;   // 選べるキャラクターと、選んでいる人(借りるだけ)
};
