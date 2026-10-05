#pragma once
#include <DirectXMath.h>

#include "Component.h"

class Ragdoll;

// 発射したラグドールを、カメラが追いかける
class CameraFollow : public Component
{
public:
	void Initialize(ServiceLocator& context)override;
	void Update(float deltaTime)override;

	// 追いかける相手(借りるだけ。所有者はシーンのGameObject)
	void SetTarget(Ragdoll* target) { _target = target; }

	void StartFollow();

	void ReturnToStart();
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

	DirectX::XMFLOAT3 _startPosition{};  // カメラの最初の位置(戻る先)
	float _startRootY = 0.0f;            // 追い始めたときの、ラグドールの腰の高さ
	bool _hasBeenFast = false;           // 追い始めてから、一度でも速くなったか(なったあとで、遅くなったら寄る)
};
