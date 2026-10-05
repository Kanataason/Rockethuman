#pragma once
#include <DirectXMath.h>

struct Transform
{
    DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 Rotation{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 Scale{ 1.0f, 1.0f, 1.0f };

    // 見た目だけの回転(度)。モデルの向きが、ゲームの向きと合わないときに、モデルだけを回す。
    // 描画の行列にだけ入り、GetForward/GetRight/GetUpや、Rotationを書き換える処理(大砲の揺れなど)には影響しない
    DirectX::XMFLOAT3 ModelRotation{ 0.0f, 0.0f, 0.0f };

    //すべての値のワールド行列を合成
    DirectX::XMMATRIX GetWorldMatrix() const;

    //現在の回転から前方向ベクトルを算出
    DirectX::XMFLOAT3 GetForward() const;

    //現在の回転から右方向ベクトルを算出
    DirectX::XMFLOAT3 GetRight() const;

    //現在の回転から上方向ベクトルを算出
    DirectX::XMFLOAT3 GetUp()const;
};