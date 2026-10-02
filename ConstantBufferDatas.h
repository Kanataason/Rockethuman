#pragma once
#include <DirectXMath.h>

using namespace DirectX;
enum class LightType:int
{
    Directional,
    Point,
    Spot
};
//GPUに送るライトデータ
struct LightData
{
    int Type;
    float Padding0[3];

    XMFLOAT3 Position;
    float Range;

    XMFLOAT3 Direction;
    float Angle;

    XMFLOAT4 Color;
};

struct MaterialData
{
    XMFLOAT4 Color;
    int Unlit;
    float Padding[3];
};