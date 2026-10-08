#pragma once

//ゲーム全体の時間の速さ(倍率)。スローモーションに使う
class TimeScale
{
public:
	//すでに、これより遅いときは、遅いほうを優先する(連続して当たっても、途中で速くならない)
	void Request(float scale, float seconds);

	//戻る動きを進める。倍率をかける前の、実際に経った時間(秒)を渡すこと
	void Update(float realDeltaTime);

	//ポーズ。trueの間は、倍率が0になり、シーンの更新も物理も止まる(スローモーションの戻りは、裏で進む)
	void SetPaused(bool paused) { _paused = paused; }

	float Get() const { return _paused ? 0.0f : _current; }
private:
	bool _paused = false;
	float _current = 1.0f; //今の倍率
	float _startScale = 1.0f; //戻り始めたときの倍率
	float _elapsed = 0.0f; //戻り始めてから経った実時間
	float _duration = 0.0f; //戻るのにかける実時間(0のときは、止まっている)
};
