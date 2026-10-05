#pragma once
#include <string>
#include <vector>

#include "BaseScene.h"
#include "ScoreManager.h"
#include "RoundManager.h"
#include "ResultPanel.h"
#include "UIRenderer.h"
#include "UiLayout.h"
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
	ResultPanel _resultPanel;   // 玉を撃ち終えたあとに、ゲーム画面の上に重ねて出す

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

	// ゲージの色の帯は、発射の速さと同じ表(PowerZone.h)を使う
	// 的に当たったときのスローモーション(倍率と、元の速さに戻るまでの実時間)
	TimeScale* _timeScale = nullptr;
	float _hitSlowScale = 0.35f;
	float _hitSlowSeconds = 1.25f;
	float _headSlowScale = 0.2f;     // 頭ヒットは、もっと遅く、長く
	float _headSlowSeconds = 1.45f;
};
