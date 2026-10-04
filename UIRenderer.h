#pragma once
#include <memory>
#include<vector>
#include <string>
#include <array>
#include <d3d11.h>
#include <wrl/client.h>

#include <DirectXMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

enum class TextAlign { Left, Center, Right };
enum class TextType
{
    Start,
    Setting,
    Select,
    End,
    MouseControll,
    Score,
    ShotCount,
    None
};
enum class ButtonType
{
    Box,
    Setting,
    End,
    Panel,
    MouseControll,
    Score,
    None
};
enum class PanelType
{
    Setting,
    Score,
    ShotCount,
    CannonGauge,
    None
};

// 上から下への並び順。W/Sで選ぶ順番になる
struct MenuEntry
{
    TextType text;
    ButtonType button;
};

struct ButtonData
{
    //ボタンの大きさ
    float left = 0.0f;
    float top = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float thickness = 2.0f;//枠の太さ

    DirectX::XMFLOAT4 frameColor = { 1, 1, 1, 1 };  //枠の色
};

struct TextData
{
    std::wstring text;

    float x = 0.0f;
    float y = 0.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float scale = 1.0f;

    DirectX::XMFLOAT4 mainColor = { 1, 1, 1, 1 };
    DirectX::XMFLOAT4 outlineColor = { 0, 0, 0, 1 };

    float outLineStiffnes = 2.0f;
};

struct PanelData
{
    ButtonData frame;                           // パネルの位置・大きさ・枠の太さ・枠の色
    DirectX::XMFLOAT4 fillColor = { 0, 0, 0, 0.6f };  // 塗りの色(アルファで透け具合)
    std::vector<MenuEntry> items;               // ボタン+文字の組
};

namespace UiInfo
{
    inline const std::unordered_map<TextType, TextData> textList =
    {
        {TextType::Start,{.text = L"スタート",.centerX = 640.0f,.centerY = 430.0f,.scale = 0.7f
        ,.mainColor = { 1, 1, 1, 1 },.outlineColor = { 0, 0, 0, 1 }}},
        {TextType::Setting,{.text = L"オプション",.centerX = 640.0f,.centerY = 510.0f,.scale = 0.55f
        ,.mainColor = { 1, 1, 1, 1 },.outlineColor = { 0, 0, 0, 1 }}},
        {TextType::End,{.text = L"エンド",.centerX = 640.0f,.centerY = 590.0f,.scale = 0.7f
        ,.mainColor = { 1, 1, 1, 1 },.outlineColor = { 0, 0, 0, 1 }}},
        {TextType::MouseControll,{.text = L"マウスのかんど",.centerX = 350.0f,.centerY = 200.0f,.scale = 0.7f
        ,.mainColor = { 1, 1, 1, 1 },.outlineColor = { 0, 0, 0, 1 }}},
         {TextType::Score,{.text = L"スコア",.centerX = 90.0f,.centerY = 44.0f,.scale = 0.3f
        ,.mainColor = { 0.7f, 0.8f, 1.0f, 1 },.outlineColor = { 0, 0, 0, 1 }}},
         {TextType::ShotCount,{.text = L"のこり",.centerX = 640.0f,.centerY = 44.0f,.scale = 0.3f
        ,.mainColor = { 0.7f, 0.8f, 1.0f, 1 },.outlineColor = { 0, 0, 0, 1 }}}
    };

    inline const std::unordered_map<ButtonType, ButtonData> buttonList =
    {
        {ButtonType::Box,{.left = 540.0f,.top = 400.0f,.width = 200.0f,.height = 60.0f}},
        {ButtonType::Setting,{.left = 540.0f,.top = 480.0f,.width = 200.0f,.height = 60.0f}},
        {ButtonType::End,{.left = 540.0f,.top = 560.0f,.width = 200.0f,.height = 60.0f}},
        {ButtonType::Panel,{.left = 350.0f,.top = 100.0f,.width = 500.0f,.height = 500.0f}},
        {ButtonType::MouseControll,{.left = 350.0f,.top = 100.0f,.width = 200.0f,.height = 200.0f}},
    };

    inline const std::unordered_map<PanelType, PanelData> panelList =
    {
        { PanelType::Setting, {
            .frame = {.left = 350.0f, .top = 100.0f, .width = 500.0f, .height = 500.0f,
                       .thickness = 2.0f, .frameColor = { 1, 1, 1, 1 } },
            .fillColor = { 0.0f, 0.0f, 0.0f, 0.6f },
            .items = {{TextType::MouseControll, ButtonType::MouseControll},
            } }
        },
        {PanelType::Score, {
            .frame = {.left = 20.0f, .top = 20.0f, .width = 320.0f, .height = 120.0f,
                       .thickness = 2.0f, .frameColor = { 1, 1, 1, 1 } },
            .fillColor = { 0.0f, 0.0f, 0.0f, 0.6f },
              .items = {{TextType::Score, ButtonType::None}
            }}
        },
        // 画面の上の中央(1280の中央 640 を、パネルの中心にする)
        {PanelType::ShotCount, {
            .frame = {.left = 520.0f, .top = 20.0f, .width = 240.0f, .height = 120.0f,
                       .thickness = 2.0f, .frameColor = { 1, 1, 1, 1 } },
            .fillColor = { 0.0f, 0.0f, 0.0f, 0.6f },
              .items = {{TextType::ShotCount, ButtonType::None}
            }}
        },
           {PanelType::CannonGauge, {
            .frame = {.left = 80.0f, .top = 200.0f, .width = 30.0f, .height = 200.0f,
                       .thickness = 2.0f, .frameColor = { 0, 1, 0, 1 } },
            .fillColor = { 0.0f, 0.0f, 0.0f, 0.6f },
              .items = {{TextType::None, ButtonType::None}
            }}
        }
    };

    const std::array<MenuEntry, 3> MenuEntries =
    { {
        { TextType::Start,   ButtonType::Box},
        { TextType::Setting, ButtonType::Setting},
        { TextType::End,     ButtonType::End}
    } };


    constexpr int MenuCount = static_cast<int>(MenuEntries.size());
}

// UI描画をまとめるクラス（DirectXTK の SpriteBatch / SpriteFont を使用）
// ・座標はすべて 1280x720 の仮想解像度で指定する（Begin() で実際の画面サイズへ拡大縮小）
// ・色はストレートアルファ (r, g, b, a) で渡せばよい（内部で乗算済みアルファに変換する）
class UiRenderer
{
public:
    static constexpr float VirtualWidth = 1280.0f;
    static constexpr float VirtualHeight = 720.0f;

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
        const wchar_t* fontFilePath, float screenWidth, float screenHeight);
    void OnResize(float screenWidth, float screenHeight);

    // 3D描画の後に Begin → 各種 Draw → End の順で呼ぶ
    void Begin();
    void End();

    void DrawRect(float left, float top, float width, float height, DirectX::FXMVECTOR color);
    void DrawFrame(float left, float top, float width, float height,
        float thickness, DirectX::FXMVECTOR color);

    // x は align に応じて「左端 / 中央 / 右端」、y は文字の上端
    void DrawString(const std::wstring& text, float x, float y, float scale,
        DirectX::FXMVECTOR color, TextAlign align = TextAlign::Left);
    void DrawStringWithOutline(const std::wstring& text, float x, float y, float scale,
        DirectX::FXMVECTOR color, DirectX::FXMVECTOR outlineColor,
        float outlineThickness = 3.0f, TextAlign align = TextAlign::Left);

    // (centerX, centerY) を中心に描く。拡大縮小アニメーションしても中心がずれない
    // outlineThickness に 0 を渡すと縁取りなし
    void DrawStringCentered(const std::wstring& text, float centerX, float centerY, float scale,
        DirectX::FXMVECTOR color, DirectX::FXMVECTOR outlineColor,
        float outlineThickness = 3.0f);

    DirectX::XMFLOAT2 MeasureString(const std::wstring& text, float scale) const;

    // ボタンの判定用:マウスの位置(ウィンドウ内のピクセル座標)が、
    // 仮想解像度(1280x720)基準の矩形(left, top, width, height)の中にあるか調べる
    // ※ clientPos は InputSystem::GetMouseClientPosition() で取得したものを渡す
    bool HitTest(float left, float top, float width, float height, const POINT& clientPos) const;

    // 3D空間の座標 → 仮想解像度上の画面座標（「頭ヒット!」をキャラの位置に出すときなどに使う）
    DirectX::XMFLOAT2 WorldToScreen(DirectX::FXMVECTOR worldPosition,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection) const;

private:
    std::unique_ptr<DirectX::SpriteBatch> _spriteBatch;
    std::unique_ptr<DirectX::SpriteFont>  _font;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _whiteTexture;
    float _screenWidth = VirtualWidth;
    float _screenHeight = VirtualHeight;
};

// 12480 → L"12,480"
inline std::wstring FormatWithComma(int value)
{
    std::wstring digits = std::to_wstring(value < 0 ? -value : value);
    for (int insertPosition = static_cast<int>(digits.size()) - 3; insertPosition > 0; insertPosition -= 3)
    {
        digits.insert(static_cast<size_t>(insertPosition), L",");
    }
    return value < 0 ? L"-" + digits : digits;
}