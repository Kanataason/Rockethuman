#pragma once
#include "ServiceLoacator.h"
#include "VectorUtility.h"
#include "Event.h"

class ObjectStatus;
struct HitInfo;

// 的に当たって、点が入ったときの情報(「+500」のようなポップアップを出すのに使う)
struct ScoreGain
{
	float points = 0.0f;    // 今回入った点(コンボ倍率・頭ボーナスを含む)
	int combo = 0;          // 今のコンボ数
	bool isHead = false;    // 頭に当たったか
	Vector3 position{};     // 当たった場所(世界座標)
};

// スコアの採点
class ScoreManager
{
public:
	// 的に当たって、点が入った瞬間
	Event<ScoreGain> OnScoreGained;

	void Initialize(ServiceLocator& locator) {};

	void SubScribeEvent(ObjectStatus* object);

	// コンボの制限時間を進める
	void Update(float deltaTime);

	// 1発ぶんの飛距離(m)を点にして足す
	void AddDistanceBonus(float meters);

	// コンボを途切れさせる(1発が終わったときに呼ぶ)
	void ResetCombo();

	float GetScore()const { return _totalScore; }
	int GetCombo()const { return _combo; }
private:
	void AddHit(const HitInfo& hit);
private:
	// ---- 採点の調整値 ----
	float _comboWindowSeconds = 3.0f;   // 前の的から、この秒数以内に当てるとコンボ
	float _comboStep = 0.25f;           // コンボ1つにつき、倍率が増える量
	float _maxMultiplier = 3.0f;        // 倍率の上限
	float _headBonus = 500.0f;          // 頭に当たったときの追加点(倍率は掛けない)
	float _pointsPerMeter = 10.0f;      // 飛距離1mあたりの点

	float _totalScore = 0.0f;
	int _combo = 0;
	float _comboTimer = 0.0f;   // 0より大きい間は、コンボが続いている
};
