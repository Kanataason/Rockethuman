#pragma once
#include <d3d11.h>
#include <Windows.h>
#include <iostream>
#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
static const unsigned SCREEN_WIDTH = 1920;
static const unsigned SCREEN_HEIGHT = 1080;

class BaseSystem
{
 
public:
    BaseSystem() {};

    IDXGIAdapter1* FindBestAdapter();
    void CreateDepthStencil();
    void CreateDeviceAndSwapChain();
    void Initialize(HWND hwnd);
    void ClearScreen();
    void Present();
    void Release();

    ID3D11Device* GetDevice() const{ return _device.Get(); }
    ID3D11DeviceContext* GetContext()const { return _context.Get(); }

    void SetScreenInfo(int width, int height, HWND hwnd);
    void SetViewPort(float width, float height, float minDeph, float maxDepth, float topX, float topY);
private:

    ComPtr<ID3D11Device> _device;
    ComPtr<ID3D11DeviceContext> _context;
    ComPtr<IDXGISwapChain> _swapChain;

    ComPtr<ID3D11RenderTargetView> _renderTargetView;
    ComPtr<ID3D11Texture2D> _backBuffer;
    ComPtr<ID3D11DepthStencilView> _depthStencil;//深度の設定view
    //描画設定を決める
    DXGI_SWAP_CHAIN_DESC _swapChainDesc = {};

    D3D11_VIEWPORT _viewport = {};

};