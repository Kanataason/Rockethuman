#pragma once

#include "MenuList.h"
#include "TitlePage.h"
#include "ServiceLocator.h"

class InputSystem;
class UiRenderer;
class CharacterRoster;

//キャラクター選択。一覧は、CharacterRoster(Characters.json)から作る
//選ぶと、すぐに「選んでいる人」が変わる(決定は、画面を閉じるだけ)。ステージを始めるときに、この人が出る
class CharacterSelectPage : public TitlePage
{
public:
	void Initialize(ServiceLocator& locator);

	void Open() override;
	void Update() override;
	void Draw() override;

private:
	InputSystem* _input = nullptr;
	UiRenderer* _uiRenderer = nullptr;
	CharacterRoster* _characters = nullptr; //借りるだけ

	MenuList _list;
};
