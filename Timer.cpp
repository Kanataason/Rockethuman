
#include "Timer.h"

void Timer::Initialize()
{
	_prevTime = std::chrono::high_resolution_clock::now();
}
float Timer::Tick()
{

    //現在時刻を取得
    auto currentTime = std::chrono::high_resolution_clock::now();

    //経過時間を計算
    std::chrono::duration<float> delta =
        currentTime - _prevTime;

    _prevTime = currentTime;

    //秒に変換して返す
    return delta.count();
}   