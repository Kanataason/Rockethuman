#pragma once
#include <wrl/client.h>
#include <d3d11.h>

#include "Event.h"
#include "ServiceLoacator.h"

class InputSystem;
class UiRenderer;

// ステージが終わったときに、ゲーム画面の上に重ねて出す結果パネル
//   暗くなる -> パネルが弾んで出る -> スコアが数え上がる -> 星が1つずつ出る -> ボタンが出る
// 「出すかどうか」「アニメーションの進み」は、このクラスが持つ(GameScene は、Show を呼ぶだけ)
class ResultPanel
{
public:
	// 「もういちど」を選んだとき / 「タイトルへ」を選んだとき
	Event<> OnRetry;
	Event<> OnBackToTitle;

	void Initialize(ServiceLocator& locator);

	// 結果を出し始める(星の数もここで決める)。毎フレーム呼んでも、2回目以降は無視する
	void Show(int finalScore);

	void Update(float deltaTime);
	void Draw();

	bool IsVisible() const { return _visible; }
private:
	// 星の数(0～3)。スコアが、何点以上で何個か
	static int CalculateStars(int score);

	// 開始から start 秒後に始まる、duration 秒のアニメーションの進み具合(0～1)
	float Progress(float start, float duration) const;

	void DrawStars(float centerX, float centerY);
	void DrawButtons(float panelTop);
private:
	InputSystem* _input = nullptr;
	UiRenderer* _uiRenderer = nullptr;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _starOn;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _starOff;

	bool _visible = false;
	float _elapsed = 0.0f;   // Show してからの秒数。アニメーションは、全部これから計算する
	int _score = 0;
	int _starCount = 0;
	int _selectedIndex = 0;  // 0: もういちど, 1: タイトルへ
};
