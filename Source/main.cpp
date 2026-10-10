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
static RECT CalculateWindowRect(DWORD windowStyle)
{
    //メインモニターの、タスクバーを除いた領域
    HMONITOR monitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
    GetMonitorInfo(monitor, &monitorInfo);
    const RECT workArea = monitorInfo.rcWork;

    //タイトルバーと枠の太さを調べる
    RECT frameRect = { 0, 0, 0, 0 };
    AdjustWindowRect(&frameRect, windowStyle, FALSE);
    const LONG frameWidth = frameRect.right - frameRect.left;
    const LONG frameHeight = frameRect.bottom - frameRect.top;

    //枠を除いて、中身に使える大きさ
    const float availableWidth = static_cast<float>(workArea.right - workArea.left - frameWidth);
    const float availableHeight = static_cast<float>(workArea.bottom - workArea.top - frameHeight);

    //16:9を保ったまま収まる倍率(1より大きくはしない)
    const float fitScale = (std::min)({ 1.0f,
        availableWidth / SCREEN_WIDTH,
        availableHeight / SCREEN_HEIGHT });

    const LONG clientWidth = static_cast<LONG>(SCREEN_WIDTH * fitScale);
    const LONG clientHeight = static_cast<LONG>(SCREEN_HEIGHT * fitScale);
    const LONG windowWidth = clientWidth + frameWidth;
    const LONG windowHeight = clientHeight + frameHeight;

    //画面の真ん中に置く
    const LONG windowLeft = workArea.left + (workArea.right - workArea.left - windowWidth) / 2;
    const LONG windowTop = workArea.top + (workArea.bottom - workArea.top - windowHeight) / 2;

    return { windowLeft, windowTop, windowLeft + windowWidth, windowTop + windowHeight };
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

    const DWORD windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    const RECT windowRect = CalculateWindowRect(windowStyle);


    //windowsを実際に生成
     HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"ロケットにんげん",
        windowStyle,
        windowRect.left,
        windowRect.top,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);


    GameManager gameManager;

    gameManager.Initialize(hwnd);


    MSG msg = {};
    ShowCursor(FALSE);//カーソル非表示

    while (msg.message != WM_QUIT)//終了するまで繰り返す
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
