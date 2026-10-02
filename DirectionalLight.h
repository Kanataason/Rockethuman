#pragma once
#include "BaseLight.h"

class DirectionalLight :public Light
{
public:

    LightType GetLightType() const override
    {
        return LightType::Directional;
    }

    //ライトの方向を取得
    void SetDirection(const DirectX::XMFLOAT3& dir)
    {
        m_Direction = dir;
    }

    const DirectX::XMFLOAT3& GetDirection() const
    {
        return m_Direction;
    }

private:
    DirectX::XMFLOAT3 m_Direction = { 0.0,-1.0,0.0 };
};