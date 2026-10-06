#pragma once
#include <DirectXMath.h>

#include "Transform.h"
#include "Object.h"
#include "Component.h"
#include "ServiceLocator.h"

class Camera:public Component
{


public:
	void Initialize(ServiceLocator& locator) override {};
	void Update(float deltaTime) override {};

	//ビュー行列を取得する
	DirectX::XMMATRIX GetViewMatrix() const;

	//射影行列を取得する
	DirectX::XMMATRIX GetProjectionMatrix() const;

private:

	float fov = DirectX::XM_PIDIV4;
	float aspect = 16.0f / 9.0f;
	float nearZ = 0.1f;
	float farZ = 1000.0f;
};