#pragma once
#include <DirectXMath.h>

#include "Component.h"
#include "ConstantBufferDatas.h"

using namespace DirectX;


class Light :public Component
{
public:
	virtual ~Light() = default;

    virtual LightType GetLightType() const = 0;

    void SetColor(const DirectX::XMFLOAT4& color)
    {
        m_Color = color;
    }

    DirectX::XMFLOAT4 GetColor() const
    {
        return m_Color;
    }

protected:
    DirectX::XMFLOAT4 m_Color = { 1,1,1,1 };
};