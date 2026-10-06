#pragma once

#include "Event.h"
#include "ServiceLoacator.h"

class InputSystem;
class UiRenderer;

// ステージ中に、ESCキーで出す、ポーズのパネル(ゲーム画面の上に重ねて出す)
//   つづける / やりなおす / タイトルへ
// 時間を止めるのは、受け取った側(GameScene)の仕事。このクラスは、「出すかどうか」「選んだもの」だけを持つ
class PausePanel
{
public:
	// 「つづける」(ESCでも同じ) / 「やりなおす」 / 「タイトルへ」を選んだとき。選んだ時点で、パネルは閉じている
	Event<> OnResume;
	Event<> OnRetry;
	Event<> OnBackToTitle;

	void Initialize(ServiceLocator& locator);

	void Open();
	bool IsVisible() const { return _visible; }

	// 出ている間だけ呼ぶ。キー入力を受け付ける
	void Update();
	void Draw();
private:
	InputSystem* _input = nullptr;
	UiRenderer* _uiRenderer = nullptr;

	bool _visible = false;
	int _selectedIndex = 0;   // 0: つづける, 1: やりなおす, 2: タイトルへ
};
