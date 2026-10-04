#pragma once
#include <string>
#include <vector>

#include "BaseScene.h"
#include "ScoreManager.h"
#include "RoundManager.h"
#include "UIRenderer.h"
#include "UiTween.h"

class Ragdoll;
class CameraFollow;
class TimeScale;

class GameScene : public BaseScene
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;
	void DrawUi()override;
private:
	void SubscribeEvents();

	void DrawPanelBase(const PanelData& panel);
	void UpdateScorePanel(const PanelData& panel);
	void UpdateGauge(float deltaTime);
	void DrawShotCountPanel();

	// 点が入ったときの演出(「+500」の浮き上がるポップアップと、COMBO表示)
	void AddScorePopup(const ScoreGain& gain);
	void UpdateScoreEffects(float deltaTime);
	void DrawScorePopups();
	void DrawComboText();
	void DrawGauge();
	void DrawAimHint();
private:
	// 的に当たった場所に出す、浮き上がって消える文字
	struct ScorePopup
	{
		Vector3 worldPosition{};
		std::wstring text;
		bool isHead = false;
		float age = 0.0f;   // 出てからの秒数
	};

	RoundManager _roundManager;
	ScoreManager _scoreManager;

	Ragdoll* _ragdoll = nullptr;
	float _launchRootX = 0.0f;   // 発射した瞬間の、体のX位置(飛距離の採点に使う)
	CameraFollow* _cameraFollow = nullptr;

	// 残りの玉数が、ポンと弾むアニメーション(ラグドールが止まったとき)
	Tween _shotCountPop;

	std::vector<ScorePopup> _scorePopups;
	Tween _comboPop;       // コンボが増えたときの弾み
	int _lastCombo = 0;    // 前のフレームのコンボ数(増えたのを見つけるため)

	//CannonPowerGauge
	float _gaugeSpeed = 2.0f;
	float _gaugeFrame = 0.0f;
	float _gaugeAmmount = 0.0f;

	// ゲージを区切る色の帯。下から順に、upToまでの範囲(0～1)をその色にする。最後のupToは1にすること
	struct GaugeZone
	{
		float upTo;
		DirectX::XMFLOAT4 color;
	};
	std::vector<GaugeZone> _gaugeZones =
	{
		{ 0.33f, { 0.0f, 1.0f, 0.0f, 1.0f } },    // 弱い: 黄色
		{ 0.8f, { 1.0f, 0.55f, 0.1f, 1.0f } },   // 中くらい: 橙色
		{ 1.0f,  { 1.0f, 0.15f, 0.1f, 1.0f } },   // 強い: 赤
	};

	// 的に当たったときのスローモーション(倍率と、元の速さに戻るまでの実時間)
	TimeScale* _timeScale = nullptr;
	float _hitSlowScale = 0.35f;
	float _hitSlowSeconds = 1.25f;
	float _headSlowScale = 0.2f;     // 頭ヒットは、もっと遅く、長く
	float _headSlowSeconds = 1.45f;
};
