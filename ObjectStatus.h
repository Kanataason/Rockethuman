#pragma once
#include "Component.h"
#include "VectorUtility.h"
#include "Event.h"

// 的に体が当たったときの情報(採点に使う)
struct HitInfo
{
	float baseScore = 0.0f;   // 的の基本点
	bool isHead = false;      // 頭が当たったか
	Vector3 position{};       // 当たった場所(ポップアップなどの表示に使う)
};

class ObjectStatus : public Component
{
public:
	Event<HitInfo> OnSendScore;

	void SetScore(float score) { _score = score; }
	void OnCollisionEnter(Collider* other, const CollisionResult& result)override;
private:
	Vector3 _initializePos;
	float _score = 100.0f;
	bool _isHit = false;   // 的は、最初に当たった1回だけ得点にする(体の部位ごとに何度も当たるため)
};
