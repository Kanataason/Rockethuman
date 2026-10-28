#pragma once
#include "ServiceLocator.h"
#include "Event.h"
#include "VectorUtility.h"

class Ragdoll;
class RoundManager;
class EffectSystem;
class InputSystem;
class UiRenderer;

//ロケット噴射。飛んでいる間に、スペースキーで1回だけ、好きな向きへ加速する
//ここが決めるのは「いつ使えるか」「どの向きか」「案内の表示」だけ。
//体を動かすのは Ragdoll、炎は EffectSystem、画面の揺れや音は、通知(OnBoosted)を受けた側が行う
class RocketBoost
{
public:
	//噴射した瞬間(画面の揺れや、音に使う)
	Event<> OnBoosted;

	void Initialize(ServiceLocator& locator);

	//飛ばす体、1発ごとの流れ、炎を出す相手をつなぐ。ragdoll は、なくてもよい(nullptr)
	void Bind(Ragdoll* ragdoll, RoundManager& roundManager, EffectSystem& effects);

	//入力を受け付けて、噴射している間は、炎を出す
	void Update(float deltaTime);

	//案内の表示。UiRenderer の Begin～End の間で呼ぶ
	void Draw();

	//いま、噴射できるか(飛んでいて、まだ使っていなくて、発射の直後ではない)
	bool CanBoost() const;

private:
	void Fire();

	//噴射する向き。いまの進行方向に、W / S（↑ / ↓）で、上下に曲げる
	Vector3 DecideDirection() const;

	//1発ごとに、使える状態に戻す
	void Reset();

private:
	InputSystem* _input = nullptr;
	UiRenderer* _uiRenderer = nullptr;
	Ragdoll* _ragdoll = nullptr;
	RoundManager* _roundManager = nullptr;
	EffectSystem* _effects = nullptr;

	bool _used = false; //この1発では、もう使った
	float _secondsSinceShot = 0.0f; //発射してからの秒数(発射のスペースが、そのまま噴射にならないように)
	float _flameRemaining = 0.0f; //炎を出す、残りの秒数
	Vector3 _direction{ 1.0f, 0.0f, 0.0f }; //噴射した向き(炎を、反対側へ出すのに使う)
};
