#pragma once
#include <vector>

#include "Event.h"
#include "VectorUtility.h"
#include "Explosive.h"

class Ragdoll;
class ObjectStatus;

//爆発の実行。爆発の依頼を受けて、半径の中の的に、距離に応じた時間差で爆風を届け(連鎖して見える)、
//人間を打ち上げる。火の玉・画面の揺れ・音は、通知(OnExploded)を受けた側が行う
//(爆発の依頼は、当たりの通知の最中に来るので、実行は、次の Update にまとめて行う)
class ExplosionManager
{
public:
	//爆発した瞬間(火の玉や、画面の揺れ、音に使う)
	Event<Explosion> OnExploded;

	//爆風を受ける的(借りるだけ)
	void AddTarget(ObjectStatus* target);

	//爆発する箱の通知を受け取る
	void Subscribe(Explosive* explosive);

	//爆風で打ち上げる体。なくてもよい(nullptr)
	void Bind(Ragdoll* ragdoll) { _ragdoll = ragdoll; }

	void Update(float deltaTime);

private:
	void Detonate(const Explosion& explosion);
	void PushRagdoll(const Explosion& explosion);

private:
	//爆風を、これから的に届ける予定
	struct PendingHit
	{
		ObjectStatus* target = nullptr;
		Vector3 origin{}; //爆心
		float delay = 0.0f; //届くまでの残り時間(秒)
	};

	std::vector<ObjectStatus*> _targets;
	std::vector<Explosion> _requests;
	std::vector<PendingHit> _pendingHits;
	Ragdoll* _ragdoll = nullptr;
};
