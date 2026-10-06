#pragma once
#include <string>

#include "ServiceLoacator.h"

class Ragdoll;
class ScoreManager;
class RoundManager;
class SoundManager;
class CharacterRoster;

//ステージ中の、出来事と音の対応。GameSceneは、通知(Event)をつなぐだけで、どの音をいつ鳴らすかは、ここが決める
//・発射した瞬間 -> "shot"      ・地面に着くたび -> "hit"      ・的に当たった瞬間 -> "hit"(連打は間引く)
//・頭に当たった -> "head"      ・クリアボーナス -> "clear"    ・玉を撃ち終えた -> ドラムロール
//選んでいるキャラクターに専用の音があれば、それを鳴らす(なければ、標準の音)
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
	SoundManager* _sound = nullptr; //借りるだけ(ゲーム全体で1つ)
	CharacterRoster* _characters = nullptr; //選んでいるキャラクター(専用の音を探すのに使う。借りるだけ)

	float _hitSeCoolDown = 0.06f; //的に当たった音の、最小の間隔(秒)
	float _currentHitSeTime = 0.0f;
};
