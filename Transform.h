#pragma once
#include <DirectXMath.h>

struct Transform
{
    DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 Rotation{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 Scale{ 1.0f, 1.0f, 1.0f };

    //すべての値のワールド行列を合成
    DirectX::XMMATRIX GetWorldMatrix() const;

    //現在の回転から前方向ベクトルを算出
    DirectX::XMFLOAT3 GetForward() const;

    //現在の開店から右方向ベクトルを算出
    DirectX::XMFLOAT3 GetRight() const;
};