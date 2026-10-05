#include "ResultPanel.h"

#include <algorithm>
#include <string>
#include <Windows.h>

#include "InputSystem.h"
#include "UIRenderer.h"
#include "UiLayout.h"
#include "UiTween.h"

using namespace UiInfo;

namespace
{
	// ---- 演出のタイミング(秒) ----
	constexpr float panelPopSeconds = 0.35f;        // パネルが弾んで出る
	constexpr float scoreCountSeconds = 0.9f;       // スコアが0から数え上がる
	constexpr float starIntervalSeconds = 0.45f;    // 星が1つずつ出る間隔
	constexpr float starPopSeconds = 0.4f;
	constexpr float buttonDelaySeconds = 0.3f;      // 最後の星が出てから、ボタンが出るまで

	constexpr float dimAlpha = 0.55f;               // 後ろのゲーム画面を暗くする濃さ
	constexpr float dimFadeSeconds = 0.3f;

	// ---- パネルの中の配置(パネルの上端からの距離。1280x720基準) ----
	constexpr float titleY = 55.0f;
	constexpr float scoreLabelY = 125.0f;
	constexpr float scoreValueY = 195.0f;
	constexpr float starsY = 310.0f;
	constexpr float starSize = 96.0f;
	constexpr float starSpacing = 120.0f;
	constexpr float buttonTop = 400.0f;
	constexpr float buttonWidth = 280.0f;
	constexpr float buttonHeight = 56.0f;
	constexpr float buttonGap = 14.0f;

	const wchar_t* const buttonLabels[2] = { L"もういちど", L"タイトルへ" };
}

void ResultPanel::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	_uiRenderer = locator.uiRenderer;

	if (_uiRenderer)
	{
		_starOn = _uiRenderer->LoadTexture(L"Asset/Texture/UI/star_on.png");
		_starOff = _uiRenderer->LoadTexture(L"Asset/Texture/UI/star_off.png");
	}
}

int ResultPanel::CalculateStars(int score)
{
	int stars = 0;
	for (const int threshold : _starThresholds)
	{
		if (score >= threshold)
			++stars;
	}
	return stars;
}

void ResultPanel::Show(int finalScore)
{
	if (_visible)
		return;

	_visible = true;
	_elapsed = 0.0f;
	_score = finalScore;
	_starCount = CalculateStars(finalScore);
	_selectedIndex = 0;
}

float ResultPanel::Progress(float start, float duration) const
{
	return std::clamp((_elapsed - start) / duration, 0.0f, 1.0f);
}

void ResultPanel::Update(float deltaTime)
{
	if (!_visible)
		return;

	_elapsed += deltaTime;

	// ボタンが出るまでは、キーを受け付けない(最後の1発でスペースを連打していても、すぐ決定されないように)
	const float buttonStart = panelPopSeconds + scoreCountSeconds
		+ starIntervalSeconds * _starCount + buttonDelaySeconds;
	if (_elapsed < buttonStart || !_input)
		return;

	if (_input->GetKeyDown('W') || _input->GetKeyDown(VK_UP))
		_selectedIndex = (_selectedIndex + 1) % 2;
	if (_input->GetKeyDown('S') || _input->GetKeyDown(VK_DOWN))
		_selectedIndex = (_selectedIndex + 1) % 2;

	if (_input->GetKeyDown(VK_SPACE) || _input->GetKeyDown(VK_RETURN))
	{
		// 1回選んだら、パネルは閉じる(シーンの切り替えは、受け取った側が予約する)
		_visible = false;
		if (_selectedIndex == 0)
			OnRetry.Invoke();
		else
			OnBackToTitle.Invoke();
	}
}

void ResultPanel::Draw()
{
	if (!_visible || !_uiRenderer)
		return;

	auto it = panelList.find(PanelType::Result);
	if (it == panelList.end())
		return;
	const PanelData& panel = it->second;

	// 後ろのゲーム画面を、少し暗くする(ゲーム画面は見えたまま)
	const float dim = dimAlpha * Progress(0.0f, dimFadeSeconds);
	_uiRenderer->DrawRect(0.0f, 0.0f, UiRenderer::VirtualWidth, UiRenderer::VirtualHeight,
		DirectX::XMVectorSet(0, 0, 0, dim));

	// パネル: 中心を基準に、少し小さいところから弾んで、元の大きさになる
	const float popT = Progress(0.0f, panelPopSeconds);
	const float scale = 0.8f + 0.2f * EaseOutBack(popT);
	const float alpha = (std::min)(1.0f, popT * 2.0f);

	const float centerX = panel.frame.left + panel.frame.width * 0.5f;
	const float centerY = panel.frame.top + panel.frame.height * 0.5f;
	const float width = panel.frame.width * scale;
	const float height = panel.frame.height * scale;
	const float left = centerX - width * 0.5f;
	const float top = centerY - height * 0.5f;

	DirectX::XMFLOAT4 fill = panel.fillColor;
	fill.w = (std::max)(fill.w, 0.8f) * alpha;   // 後ろが透けすぎて、文字が読みにくくならないように
	DirectX::XMFLOAT4 edge = panel.frame.frameColor;
	edge.w *= alpha;
	_uiRenderer->DrawRect(left, top, width, height, XMLoadFloat4(&fill));
	_uiRenderer->DrawFrame(left, top, width, height, panel.frame.thickness, XMLoadFloat4(&edge));

	// パネルが出きるまでは、中身は出さない
	if (_elapsed < panelPopSeconds)
		return;

	const auto white = DirectX::XMVectorSet(1, 1, 1, 1);
	const auto black = DirectX::XMVectorSet(0, 0, 0, 1);

	_uiRenderer->DrawStringCentered(L"リザルト", centerX, panel.frame.top + titleY, 0.7f, white, black);
	_uiRenderer->DrawStringCentered(L"スコア", centerX, panel.frame.top + scoreLabelY, 0.35f,
		DirectX::XMVectorSet(0.7f, 0.8f, 1.0f, 1.0f), black);

	// スコアは、0から数え上がる(最初が速く、だんだん遅くなる)
	const float countT = EaseOutCubic(Progress(panelPopSeconds, scoreCountSeconds));
	const int shownScore = static_cast<int>(_score * countT);
	_uiRenderer->DrawStringCentered(FormatWithComma(shownScore), centerX, panel.frame.top + scoreValueY,
		0.9f, white, black);

	DrawStars(centerX, panel.frame.top + starsY);
	DrawButtons(panel.frame.top);
}

void ResultPanel::DrawStars(float centerX, float centerY)
{
	const float starsStart = panelPopSeconds + scoreCountSeconds;
	const auto white = DirectX::XMVectorSet(1, 1, 1, 1);

	for (int i = 0; i < 3; i++)
	{
		const float x = centerX + (i - 1) * starSpacing;

		// 取れなかった星は、最初から暗い星を出しておく
		if (i >= _starCount)
		{
			_uiRenderer->DrawTexture(_starOff.Get(), x - starSize * 0.5f, centerY - starSize * 0.5f,
				starSize, starSize, white);
			continue;
		}

		// 取れた星は、順番に、大きく弾んで出る
		const float t = Progress(starsStart + starIntervalSeconds * i, starPopSeconds);
		if (t <= 0.0f)
		{
			_uiRenderer->DrawTexture(_starOff.Get(), x - starSize * 0.5f, centerY - starSize * 0.5f,
				starSize, starSize, white);
			continue;
		}

		const float size = starSize * (0.3f + 0.7f * EaseOutBack(t));
		_uiRenderer->DrawTexture(_starOn.Get(), x - size * 0.5f, centerY - size * 0.5f, size, size, white);
	}
}

void ResultPanel::DrawButtons(float panelTop)
{
	const float buttonsStart = panelPopSeconds + scoreCountSeconds
		+ starIntervalSeconds * _starCount + buttonDelaySeconds;
	const float alpha = Progress(buttonsStart, 0.25f);
	if (alpha <= 0.0f)
		return;

	auto it = panelList.find(PanelType::Result);
	const float centerX = it->second.frame.left + it->second.frame.width * 0.5f;

	for (int i = 0; i < 2; i++)
	{
		const float left = centerX - buttonWidth * 0.5f;
		const float top = panelTop + buttonTop + (buttonHeight + buttonGap) * i;
		const bool selected = (i == _selectedIndex);

		_uiRenderer->DrawFrame(left, top, buttonWidth, buttonHeight, 2.0f,
			selected ? DirectX::XMVectorSet(1, 1, 0, alpha) : DirectX::XMVectorSet(1, 1, 1, alpha * 0.8f));
		_uiRenderer->DrawStringCentered(buttonLabels[i], centerX, top + buttonHeight * 0.5f, 0.45f,
			selected ? DirectX::XMVectorSet(1, 1, 0, alpha) : DirectX::XMVectorSet(1, 1, 1, alpha),
			DirectX::XMVectorSet(0, 0, 0, alpha));
	}
}
