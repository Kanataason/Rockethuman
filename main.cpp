#include <d3d11.h>
#define NOMINMAX
#include <Windows.h>

#include "GameManager.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

///<summary>
///Windowのメッセージ処理
///</summary>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY://ウィンドウが閉じたら終わらせる
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int nCmdShow)
{
    //画面の拡大率の影響で、マウス座標とウィンドウサイズがズレるのを防ぐ
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    //XAudio2(サウンド)とWICは、COMが必要。すでに別のモードで初期化済みでも、そのまま続けてよい
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    //設計図を宣言
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;//コールバックの関数指定
    wc.hInstance = hInstance;//ハンドルの取得
    wc.lpszClassName = L"MyWindowClass";//アプリケーションの名前

    //設計図をwindows Osに登録
    RegisterClass(&wc);

    RECT windowRect = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    //windowsを実際に生成
    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"DirectX11",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left, //補正後の、外側の幅
        windowRect.bottom - windowRect.top, //補正後の、外側の高さ
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);


    GameManager gameManager;

    gameManager.Initialize(hwnd);


    MSG msg = {};
    ShowCursor(TRUE);//カーソル非表示

    while (msg.message != WM_QUIT)//アプリを終了したら
    {
        //windowsから何か動作連絡が来ているか確認
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        else
        {

            gameManager.Update();
        }
    }
    ShowCursor(TRUE);

    gameManager.Release();

    UnregisterClass(wc.lpszClassName, hInstance);

    if (SUCCEEDED(comResult))
        CoUninitialize();

    return static_cast<int>(msg.wParam);
}
