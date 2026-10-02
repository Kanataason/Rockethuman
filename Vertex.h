#pragma once
#include <DirectXMath.h>

using namespace DirectX;

//Meshを構成する要素　GPU側と同じように
struct Vertex
{
    DirectX::XMFLOAT3 Position = XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT3 Normal = XMFLOAT3(0, 0, 0);
    DirectX::XMFLOAT2 UV = XMFLOAT2(0, 0);

    DirectX::XMUINT4 BoneIndex = XMUINT4(0, 0, 0, 0);
    DirectX::XMFLOAT4 Weight = XMFLOAT4(0, 0, 0, 0);
};
struct DebagDrawVertex
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};