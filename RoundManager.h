#pragma once
#include "ServiceLoacator.h"
#include "Event.h"

// 1発ごとの流れ:  Aiming(狙う) -> Flying(飛んでいる) -> Result(止まって、結果を見せる) -> Aiming...
// 玉が尽きたら Finished
enum class RoundState
{
	Aiming,
	Flying,
	Result,
	Finished
};

class RoundManager
{
public:
	// ラグドールが止まった瞬間(UIのアニメーションを始めるのに使う)
	Event<> OnShotSettled;
	// 結果を見せ終わって、次の1発に進めるとき(ラグドールを大砲に戻すのに使う)
	Event<> OnReadyNextShot;
	// 玉を全部撃ち終えて、止まったあと
	Event<> OnFinished;

	void Initialize(ServiceLocator& locator);
	void Update(float deltaTime);

	// 狙っているときだけ撃てる。撃てたらtrue
	bool TryUseShot();

	// ラグドールが止まったことを伝える(飛んでいるときだけ受け付ける)
	void NotifySettled();

	int GetShotCount()const { return _shotCount; }
	RoundState GetState()const { return _state; }
private:
	// 止まってから、次に進むまでの間(秒)
	static constexpr float kResultSeconds = 1.5f;

	int _shotCount = 3;
	RoundState _state = RoundState::Aiming;
	float _resultTimer = 0.0f;
};
