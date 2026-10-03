#include "GameScene.h"

#include "CannonController.h"
#include "Player.h"
#include "ObjectStatus.h"

using namespace UiInfo;

void GameScene::Initialize(ServiceLocator& locator)
{
	BaseScene::Initialize(locator);

	auto* cannon = FindGameObject<CannonController>();
	auto* player = FindGameObject<Player>();

	if (cannon)
	{
		cannon->SetRoundManager(&_roundManager);
		cannon->SetInitializePlayer(player);
	}

	for (const auto& obj : _objects)
	{
		if (auto* component = obj->GetComponent<ObjectStatus>())
			_scoreManager.SubScribeEvent(component);
	}
}

void GameScene::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);
}
void GameScene::DrawUi()
{
	// Begin～Endの間でreturnすると、Endが呼ばれず描画の状態が壊れるので、先に探しておく
	auto it = panelList.find(PanelType::Score);
	if (it == panelList.end())return;
	const PanelData& panel = it->second;

	_uiRenderer->Begin();

	UpdateScorePanel(panel);
	DrawShotCountPanel();

	_uiRenderer->End();
}

// パネルの塗り・枠・見出しの文字を描く(スコアと残りの数で共通)
void GameScene::DrawPanelBase(const PanelData& panel)
{
	_uiRenderer->DrawRect(panel.frame.left, panel.frame.top, panel.frame.width, panel.frame.height,
		XMLoadFloat4(&panel.fillColor));
	_uiRenderer->DrawFrame(panel.frame.left, panel.frame.top, panel.frame.width, panel.frame.height,
		panel.frame.thickness, XMLoadFloat4(&panel.frame.frameColor));

	for (const auto& item : panel.items)
	{
		auto textIterator = textList.find(item.text);
		if (textIterator == textList.end())
			continue;

		const auto& textData = textIterator->second;
		_uiRenderer->DrawStringCentered(textData.text, textData.centerX, textData.centerY, textData.scale,
			XMLoadFloat4(&textData.mainColor), XMLoadFloat4(&textData.outlineColor));
	}
}

// 画面の上の中央に、残りの発数を出す
void GameScene::DrawShotCountPanel()
{
	auto it = panelList.find(PanelType::ShotCount);
	if (it == panelList.end())
		return;
	const PanelData& panel = it->second;

	DrawPanelBase(panel);

	constexpr float valueScale = 0.55f;
	constexpr float valueTop = 58.0f;
	const float centerX = panel.frame.left + panel.frame.width * 0.5f;
	_uiRenderer->DrawString(std::to_wstring(_roundManager.GetShotCount()), centerX, valueTop, valueScale,
		DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), TextAlign::Center);
}

void GameScene::UpdateScorePanel(const PanelData& panel)
{
	DrawPanelBase(panel);

	// 位置は固定なのでここで設定,
	// 現在のスコア
	constexpr float valueScale = 0.55f;
	constexpr float valueTop = 58.0f;
	constexpr float rightPadding = 20.0f;
	const std::wstring scoreText = FormatWithComma(static_cast<int>(_scoreManager.GetScore()));
	_uiRenderer->DrawString(scoreText, panel.frame.left + panel.frame.width - rightPadding, valueTop, valueScale,
		DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), TextAlign::Right);
}