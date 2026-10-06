#pragma once
#include <string>

#include "ServiceLocator.h"

class Ragdoll;
class ScoreManager;
class RoundManager;
class SoundManager;
class CharacterRoster;

//ステージ中の、出来事と音の対応。GameSceneは、通知(Event)をつなぐだけで、どの音をいつ鳴らすかは、ここが決める
class GameAudio
{
public:
	void Initialize(ServiceLocator& locator);

	//各システムの通知に、音をつなぐ。ragdoll は、なくてもよい(nullptr)
	void Bind(Ragdoll* ragdoll, ScoreManager& score, RoundManager& round);

	//当たり音の間引きの時間を進める
	void Update(float deltaTime);

private:
	//出来事("shot" "hit" "head" "clear")の効果音を鳴らす。専用の音が、設定されていない・読み込めていないときは、標準の音に戻す
	void PlayCharacterSe(const std::string& eventName);

	//的に当たった音。短い間に何個も当たっても、重なって割れないように、間引く
	void PlayTargetHitSe();

private:
	SoundManager* _sound = nullptr; //借りるだけ
	CharacterRoster* _characters = nullptr; //選んでいるキャラクター

	float _hitSeCoolDown = 0.06f; //的に当たった音の、最小の間隔(秒)
	float _currentHitSeTime = 0.0f;
};
