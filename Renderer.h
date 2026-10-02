#pragma once
#define NOMINMAX
#include <vector>
#include <Windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <wrl.h>

#include "BaseCamera.h"
#include "BaseLight.h"
#include "Transform.h"
#include "ConstantBufferDatas.h"
#include "DirectionalLight.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

using namespace DirectX;
/// <summary>
/// 描写する関数
/// </summary>

class GameObject;
class Mesh;
class Material;

class Renderer
{
	struct ConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;

		LightData light;
		MaterialData material;
	};
public:
	Renderer() = default;
	void Initialize(ID3D11Device* device,ID3D11DeviceContext* context);

	void SetAnimationBone(const std::vector<XMFLOAT4X4>& bonebuffer);
	void SetCamera(const Camera& camera);
	void SetLight(const Light& light);
	void Draw(const Mesh& mesh,const Material& material,const Transform& transform);
private:
	//コンパイル結果を確認
	bool CompileShader();

	// シェーダーオブジェクトの作成
	void CreateShader();

	void CreateConstantBuffer();
	void CreateAnimationBoneBuffer();
	void CreateRasterizer();
	void CreateSampleState();

	//GPUに渡す情報のアップデート
	void UpdateConstantBuffer(const Material& material, const Transform& transform);
	bool CheckCompileFile(HRESULT hr, ID3DBlob* errorBlob);

	// 頂点入力レイアウトの作成
	void CreateInputLayout();

	void SetDeviceAndContext(ID3D11Device* device, ID3D11DeviceContext* context)
	{
		_device = device;_context = context;
	}
private:
	ID3D11Device* _device = nullptr;
	ID3D11DeviceContext* _context = nullptr;

	ConstantBuffer _ConstantBufferData = {};

	ComPtr<ID3DBlob> _verticesRenderer;
	ComPtr<ID3DBlob> _pixelRenderer;

	ComPtr<ID3D11VertexShader> _verticesShader;
	ComPtr<ID3D11PixelShader> _pixelShader;

	ComPtr<ID3D11InputLayout> _inputLayout;
	ComPtr<ID3D11RasterizerState> _rasterizer;

	ComPtr<ID3D11ShaderResourceView> _boneSRVBuffer;
	ComPtr<ID3D11Buffer> _constantBuffer;
	ComPtr<ID3D11Buffer> _boneBuffer;
	ComPtr<ID3D11SamplerState> _sampleState;

};