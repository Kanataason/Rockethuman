#pragma once
#include <DirectXMath.h>

#include "Component.h"
#include "VectorUtility.h"
#include "Event.h"

class PhysicsSystem;

//的に体が当たったときの情報(採点に使う)
struct HitInfo
{
	float baseScore = 0.0f; //的の基本点
	bool isHead = false; //頭が当たったか
	Vector3 position{}; //当たった場所(ポップアップなどの表示に使う)
};

//点が入る的。体が当たったら、得点を伝えて、回りながら飛んでいって、小さくなって消える
class ObjectStatus : public Component
{
public:
	Event<HitInfo> OnSendScore;

	void Initialize(ServiceLocator& locator)override;
	void Update(float deltaTime)override;

	void SetScore(float score) { _score = score; }
	void OnCollisionEnter(Collider* other, const CollisionResult& result)override;
private:
	//飛んでいく動きの、状態
	enum class FlyState
	{
		None, //まだ当たっていない
		Requested, //当たった。物理から外して、飛び始めるのを待っている
		Flying, //飛んでいる
		Done //飛び終えて、消えた
	};

	void InitializeFly(float directionX);
	void UpdateFly(float deltaTime);
private:
	Vector3 _initializePos;
	float _score = 100.0f;
	bool _isHit = false; //的は、最初に当たった1回だけ得点にする(体の部位ごとに何度も当たるため)

	PhysicsSystem* _physicsSystem = nullptr;

	//---- 飛んでいく動きの調整値 ----
	float _flySpeed = 10.0f; //横の速さ(m/s)。ランダムで0.7～1.3倍にばらつく
	float _flyUpSpeed = 7.0f; //上向きの速さ(m/s)。同じくばらつく
	float _flySpinDegrees = 540.0f; //回る速さ(度/秒)。向きは、ランダム
	float _flySeconds = 1.0f; //飛び始めてから、消えるまでの秒数

	FlyState _flyState = FlyState::None;
	DirectX::XMFLOAT3 _flyVelocity{}; //今の速度
	DirectX::XMFLOAT3 _baseScale{}; //飛び始めたときの大きさ(小さくしていく元)
	float _spinDegrees = 0.0f; //今回の回る速さ(符号つき)
	float _flyAge = 0.0f; //飛び始めてからの秒数
};
