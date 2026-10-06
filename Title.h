#pragma once
#include "BaseScene.h"
#include "TitleMenuPage.h"
#include "StageSelectPage.h"
#include "CharacterSelectPage.h"

class StageRoster;

//タイトル画面。いまどの画面(メニュー / ステージ選択 / キャラクター選択)を出しているかだけを持つ
//入力と描画は、それぞれの画面(TitlePage)が受け持つ。Title は、画面どうしのつなぎと、ステージの開始・終了をする
class Title : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	void SubscribeEvents();
private:
	//画面を切り替える(開くたびに、一覧の中身と選んでいる番号を、最新に合わせる)
	void OpenPage(TitlePage& page);
private:
	TitleMenuPage _menuPage;
	StageSelectPage _stageSelectPage;
	CharacterSelectPage _characterSelectPage;
	TitlePage* _currentPage = nullptr;

	StageRoster* _stages = nullptr; //遊べるステージと、選んでいるステージ(借りるだけ)
};
