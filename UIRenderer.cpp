#include "UIRenderer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace
{
    // SpriteBatch の既定ブレンドは乗算済みアルファなので、RGB にアルファを掛けておく
    XMVECTOR ToPremultiplied(FXMVECTOR straightColor)
    {
        XMFLOAT4 color;
        XMStoreFloat4(&color, straightColor);
        return XMVectorSet(color.x * color.w, color.y * color.w, color.z * color.w, color.w);
    }
}

bool UIRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
    const wchar_t* fontFilePath, float screenWidth, float screenHeight)
{
    _spriteBatch = std::make_unique<SpriteBatch>(context);

    // フォントファイルが見つからない場合は例外が投げられる
    _font = std::make_unique<SpriteFont>(device, fontFilePath);

    // フォントに含まれない文字を描くと例外になるので、代わりに '?' を表示させる
    _font->SetDefaultCharacter(L'?');

    // 四角形（パネル・ゲージ）描画用の 1x1 の白テクスチャ
    const uint32_t whitePixel = 0xFFFFFFFF;

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = 1;
    textureDesc.Height = 1;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = &whitePixel;
    initialData.SysMemPitch = sizeof(whitePixel);

    ComPtr<ID3D11Texture2D> whiteTexture;
    if (FAILED(device->CreateTexture2D(&textureDesc, &initialData, whiteTexture.GetAddressOf())))
    {
        return false;
    }
    if (FAILED(device->CreateShaderResourceView(whiteTexture.Get(), nullptr,
        _whiteTexture.ReleaseAndGetAddressOf())))
    {
        return false;
    }

    OnResize(screenWidth, screenHeight);
    return true;
}

void UIRenderer::OnResize(float screenWidth, float screenHeight)
{
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
}

void UIRenderer::Begin()
{
    // 仮想解像度 -> 実際の画面サイズへの拡大縮小
    const XMMATRIX virtualToScreen = XMMatrixScaling(
        _screenWidth / VirtualWidth, _screenHeight / VirtualHeight, 1.0f);

    _spriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, nullptr, nullptr, nullptr,
        virtualToScreen);
}

void UIRenderer::End()
{
    _spriteBatch->End();
}

void UIRenderer::DrawRect(float left, float top, float width, float height, FXMVECTOR color)
{
    // 1x1 の白テクスチャを width x height に引き伸ばして描く
    _spriteBatch->Draw(_whiteTexture.Get(), XMFLOAT2(left, top), nullptr,
        ToPremultiplied(color), 0.0f, XMFLOAT2(0.0f, 0.0f),
        XMFLOAT2(width, height));
}

void UIRenderer::DrawFrame(float left, float top, float width, float height,
    float thickness, FXMVECTOR color)
{
    DrawRect(left, top, width, thickness, color);                                       // 上
    DrawRect(left, top + height - thickness, width, thickness, color);                  // 下
    DrawRect(left, top + thickness, thickness, height - thickness * 2.0f, color);       // 左
    DrawRect(left + width - thickness, top + thickness, thickness,
        height - thickness * 2.0f, color);                                         // 右
}

void UIRenderer::DrawString(const std::wstring& text, float x, float y, float scale,
    FXMVECTOR color, TextAlign align)
{
    const XMFLOAT2 textSize = MeasureString(text, scale);

    float drawX = x;
    if (align == TextAlign::Center) { drawX -= textSize.x * 0.5f; }
    else if (align == TextAlign::Right) { drawX -= textSize.x; }

    _font->DrawString(_spriteBatch.get(), text.c_str(), XMFLOAT2(drawX, y),
        ToPremultiplied(color), 0.0f, XMFLOAT2(0.0f, 0.0f), scale);
}

void UIRenderer::DrawStringWithOutline(const std::wstring& text, float x, float y, float scale,
    FXMVECTOR color, FXMVECTOR outlineColor,
    float outlineThickness, TextAlign align)
{
    // 8方向にずらして縁の色で描いてから、本体を上に描く
    static const XMFLOAT2 outlineDirections[] =
    {
        { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f },
        { -1.0f,  0.0f },                  { 1.0f,  0.0f },
        { -1.0f,  1.0f }, { 0.0f,  1.0f }, { 1.0f,  1.0f },
    };

    for (const XMFLOAT2& direction : outlineDirections)
    {
        DrawString(text, x + direction.x * outlineThickness, y + direction.y * outlineThickness,
            scale, outlineColor, align);
    }
    DrawString(text, x, y, scale, color, align);
}

void UIRenderer::DrawStringCentered(const std::wstring& text, float centerX, float centerY,
    float scale, FXMVECTOR color, FXMVECTOR outlineColor,
    float outlineThickness)
{
    const XMFLOAT2 textSize = MeasureString(text, scale);
    const float top = centerY - textSize.y * 0.5f;

    if (outlineThickness > 0.0f)
    {
        DrawStringWithOutline(text, centerX, top, scale, color, outlineColor,
            outlineThickness, TextAlign::Center);
    }
    else
    {
        DrawString(text, centerX, top, scale, color, TextAlign::Center);
    }
}

XMFLOAT2 UIRenderer::MeasureString(const std::wstring& text, float scale) const
{
    const XMVECTOR textSize = _font->MeasureString(text.c_str());
    return XMFLOAT2(XMVectorGetX(textSize) * scale, XMVectorGetY(textSize) * scale);
}

bool UIRenderer::HitTest(float left, float top, float width, float height, const POINT& clientPos) const
{
    // 実際の画面(ウィンドウ)座標 → 仮想解像度(1280x720)の座標へ変換する
    // (Begin() で使っている virtualToScreen の、ちょうど逆の計算)
    const float virtualX = clientPos.x * (VirtualWidth / _screenWidth);
    const float virtualY = clientPos.y * (VirtualHeight / _screenHeight);

    return virtualX >= left && virtualX <= left + width &&
           virtualY >= top && virtualY <= top + height;
}

XMFLOAT2 UIRenderer::WorldToScreen(FXMVECTOR worldPosition,
    const XMMATRIX& view, const XMMATRIX& projection) const
{
    const XMVECTOR screenPosition = XMVector3Project(
        worldPosition,
        0.0f, 0.0f, VirtualWidth, VirtualHeight, 0.0f, 1.0f,
        projection, view, XMMatrixIdentity());

    return XMFLOAT2(XMVectorGetX(screenPosition), XMVectorGetY(screenPosition));
}