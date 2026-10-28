#pragma once
#include <DirectXMath.h>
#include <random>

#include "Component.h"

class Ragdoll;

//発射したラグドールを、カメラが追いかける
class CameraFollow : public Component
{
public:
	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	//追いかける相手(借りるだけ。所有者はシーンのGameObject)
	void SetTarget(Ragdoll* target) { _target = target; }

	void StartFollow();

	void ReturnToStart();

	//画面を揺らす。strength:最初の揺れの大きさ(m) / seconds:揺れが収まるまでの秒数(だんだん弱くなる)
	void Shake(float strength, float seconds);
private:
	enum class Mode
	{
		Idle,
		Follow,
		Return
	};
private:
	float _followSharpness = 3.0f;//追う時の滑らかさ
	float _returnSharpness = 4.0f;//元のポジションに戻るときの滑らかさ
	float _lockX = 0.5f;//どこを基準にして追いかけるか
	float _followYRatio = 0.5f;//Y軸方向の追いかけ度
	float _returnFinishDistance = 0.05f;//戻るときの判定
	float _pullBackScale = 1.8f;//最大で何倍まで引くか
	float _pullBackFullSpeed = 25.0f;//この速さ(m/s)で、最大まで引く。遅いほど、引き具合も小さくなる
	float _zoomInScale = 0.3f;//止まりかけのときの倍率。1より小さいと、最初の距離より寄る
	float _zoomInArmSpeed = 8.0f;//一度この速さを超えたあとで、寄り始める(発射の直後に寄らないため)
	float _minDistanceToTarget = 8.0f;//寄っても、プレイヤーとの距離がこれより近くならない(m)

	Ragdoll* _target = nullptr;
	Mode _mode = Mode::Idle;


	DirectX::XMFLOAT3 _startPosition{}; //カメラの最初の位置(戻る先)
	float _startRootY = 0.0f; //追い始めたときの、ラグドールの腰の高さ
	float _offsetY = -2.0f;
	bool _hasBeenFast = false; //追い始めてから、一度でも速くなったか(なったあとで、遅くなったら寄る)

	//画面の揺れ。毎フレーム、前の揺れを取り除いてから、新しい揺れを足す(追いかけの計算に、混ざらないように)
	DirectX::XMFLOAT2 _shakeOffset{ 0.0f, 0.0f };
	float _shakeStrength = 0.0f;
	float _shakeSeconds = 0.0f;
	float _shakeAge = 0.0f;
	std::mt19937 _shakeRandom{ std::random_device{}() };
};
