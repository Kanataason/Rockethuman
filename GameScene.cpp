#include "GameScene.h"

#include <algorithm>
#include <cmath>

#include "CannonController.h"
#include "Player.h"
#include "Ragdoll.h"
#include "CameraFollow.h"
#include "BaseCamera.h"
#include "ObjectStatus.h"
#include "TimeScale.h"
#include "PowerZone.h"

using namespace UiInfo;

namespace
{
	// ---- 点が入ったときの演出の調整値 ----
	constexpr float popupLifetime = 1.3f;       // 「+500」が消えるまでの秒数
	constexpr float popupRisePixels = 90.0f;    // 浮き上がる高さ(1280x720基準のピクセル)
	constexpr float popupBaseScale = 0.45f;     // 文字の大きさ
	constexpr float popupPopSeconds = 0.25f;    // 出た瞬間に弾む時間
	constexpr float popupFadeStart = 0.6f;      // 寿命のここから先で、だんだん透明になる(0～1)

	// COMBO表示(スコアのパネルの下)
	constexpr float comboCenterX = 180.0f;
	constexpr float comboCenterY = 172.0f;
	constexpr float comboScale = 0.5f;
	constexpr float comboPopSeconds = 0.3f;

	// 威力ゲージ
	constexpr float dimZoneAlpha = 0.28f;        // まだ届いていない色の帯の、薄さ
	constexpr float gaugeLabelOffsetY = 22.0f;   // ゲージの下端から、帯の名前までの距離
	constexpr float gaugeLabelScale = 0.4f;
}

void GameScene::Initialize(ServiceLocator& locator)
{
	BaseScene::Initialize(locator);

	_timeScale = locator.timeScale;

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

	SubscribeEvents();
}
void GameScene::SubscribeEvents()
{
	// ラグドールが止まったら、1発ぶんの流れを進める
	//   止まった瞬間 -> 残りの玉数のアニメーション
	//   少し待つ     -> ラグドールを大砲に戻して、次の1発へ
	_ragdoll = FindGameObject<Ragdoll>();
	if (_ragdoll)
		_ragdoll->OnSettled.Subscribe([this]() { _roundManager.NotifySettled(); });

	// カメラ: 発射したらラグドールを追いかけ、次の1発の準備ができたら最初の位置へ戻る
	_cameraFollow = FindGameObject<CameraFollow>();
	if (_cameraFollow)
		_cameraFollow->SetTarget(_ragdoll);

	// 点が入ったら、当たった場所に500を出す
	_scoreManager.OnScoreGained.Subscribe([this](ScoreGain gain)
		{
			AddScorePopup(gain);

			// 当たった瞬間に、少し遅くする(頭ヒットは、もっと遅く)
			if (_timeScale)
			{
				if (gain.isHead)
					_timeScale->Request(_headSlowScale, _headSlowSeconds);
				else
					_timeScale->Request(_hitSlowScale, _hitSlowSeconds);
			}
		});

	_roundManager.OnShotFired.Subscribe([this]()
		{
			// 発射した位置を覚えておく(止まったときの飛距離の採点に使う)
			if (_ragdoll)
				_launchRootX = _ragdoll->GetRootPosition().x;
			if (_cameraFollow)
				_cameraFollow->StartFollow();
		});
	_roundManager.OnShotSettled.Subscribe([this]()
		{
			_shotCountPop.Start(0.5f);

			// 1発が終わったら、飛んだ距離を点にして、コンボを途切れさせる
			if (_ragdoll)
				_scoreManager.AddDistanceBonus(std::abs(_ragdoll->GetRootPosition().x - _launchRootX));
			_scoreManager.ResetCombo();
		});
	_roundManager.OnReadyNextShot.Subscribe([this]()
		{
			if (_ragdoll)
				_ragdoll->StopRagdoll(false);
			if (_cameraFollow)
				_cameraFollow->ReturnToStart();
		});
}

void GameScene::Update(float deltaTime)
{
	BaseScene::Update(deltaTime);

	_roundManager.Update(deltaTime);
	_scoreManager.Update(deltaTime);
	_shotCountPop.Update(deltaTime);
	UpdateGauge(deltaTime);
	UpdateScoreEffects(deltaTime);
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
	DrawComboText();
	DrawScorePopups();
	DrawGauge();
	DrawAimHint();

	_uiRenderer->End();
}

// 画面の下に、いま何を決めるのかを出す(角度 -> 威力の2段階)
void GameScene::DrawAimHint()
{
	std::wstring hint;
	switch (_roundManager.GetState())
	{
	case RoundState::AimAngle:
		hint = L"スペースキーで かくどを きめる";
		break;
	case RoundState::AimPower:
		hint = L"スペースキーで いりょくを きめる";
		break;
	default:
		return;
	}

	_uiRenderer->DrawStringCentered(hint, 640.0f, 600.0f, 0.4f,
		DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
}

void GameScene::DrawGauge()
{
	auto it = panelList.find(PanelType::CannonGauge);
	if (it == panelList.end())
		return;

	auto& panelData = it->second.frame;

	// 背景
	_uiRenderer->DrawRect(panelData.left, panelData.top, panelData.width,
		panelData.height, DirectX::XMVectorSet(0, 0, 0, 0.6f));

	// 中身: 量に応じて、下端から上へ伸びる。色の帯ごとに区切って塗る
	// 帯の位置は固定。まだ届いていない帯も薄く見せて、「上へ行くほど強い」が先にわかるようにする
	float zoneStart = 0.0f;
	for (const auto& zone : PowerZone::zones)
	{
		const float zoneHeight = panelData.height * (zone.upTo - zoneStart);
		const float zoneTop = panelData.top + panelData.height * (1.0f - zone.upTo);

		DirectX::XMFLOAT4 dimColor = zone.color;
		dimColor.w = dimZoneAlpha;
		_uiRenderer->DrawRect(panelData.left, zoneTop, panelData.width, zoneHeight, XMLoadFloat4(&dimColor));

		const float filledEnd = (std::min)(_gaugeAmmount, zone.upTo);
		if (filledEnd > zoneStart)
		{
			const float segmentHeight = panelData.height * (filledEnd - zoneStart);
			const float segmentTop = panelData.top + panelData.height * (1.0f - filledEnd);
			_uiRenderer->DrawRect(panelData.left, segmentTop, panelData.width, segmentHeight,
				XMLoadFloat4(&zone.color));
		}

		// 帯の境目に、白い線を引く(一番上の境目は枠と重なるので、引かない)
		if (zone.upTo < 1.0f)
			_uiRenderer->DrawRect(panelData.left, zoneTop - 1.0f, panelData.width, 2.0f,
				DirectX::XMVectorSet(1, 1, 1, 0.8f));

		zoneStart = zone.upTo;
	}
	_uiRenderer->DrawFrame(panelData.left, panelData.top, panelData.width,
		panelData.height, 2.0f, DirectX::XMVectorSet(1, 1, 1, 1));

	// 威力を決めている間と、発射したあとは、いまの帯の名前を、その色でゲージの下に出す
	if (_roundManager.GetState() != RoundState::AimAngle)
	{
		const PowerZone::Zone& zone = PowerZone::ZoneAt(_gaugeAmmount);
		_uiRenderer->DrawStringCentered(zone.label,
			panelData.left + panelData.width * 0.5f, panelData.top + panelData.height + gaugeLabelOffsetY,
			gaugeLabelScale, XMLoadFloat4(&zone.color), DirectX::XMVectorSet(0, 0, 0, 1));
	}
}

void GameScene::UpdateGauge(float deltaTime)
{
	switch (_roundManager.GetState())
	{
	case RoundState::AimAngle:
		// 角度を決めている間は、ゲージを空にして待つ(次の1発も、毎回0から動き出す)
		_gaugeFrame = 0.0f;
		_gaugeAmmount = 0.0f;
		break;

	case RoundState::AimPower:
		_gaugeFrame += _gaugeSpeed * deltaTime;
		_gaugeAmmount = std::abs(std::sin(_gaugeFrame));
		break;

	default:
		// 発射したあとは、止めた量のまま表示しておく
		break;
	}

	// 大砲が撃つときに読めるように、今の量を渡しておく
	_roundManager.SetPowerAmount(_gaugeAmmount);
}


void GameScene::AddScorePopup(const ScoreGain& gain)
{
	ScorePopup popup;
	popup.worldPosition = gain.position;
	popup.isHead = gain.isHead;

	const std::wstring points = L"+" + FormatWithComma(static_cast<int>(gain.points));
	popup.text = gain.isHead ? L"頭ヒット! " + points : points;

	_scorePopups.push_back(popup);
}

void GameScene::UpdateScoreEffects(float deltaTime)
{
	for (auto& popup : _scorePopups)
		popup.age += deltaTime;

	std::erase_if(_scorePopups, [](const ScorePopup& popup) { return popup.age >= popupLifetime; });

	// コンボが増えた瞬間に、COMBOの文字を弾ませる
	const int combo = _scoreManager.GetCombo();
	if (combo > _lastCombo)
		_comboPop.Start(comboPopSeconds);
	_lastCombo = combo;
	_comboPop.Update(deltaTime);
}

// 当たった場所から浮き上がって、消える文字。当たった場所は世界座標なので、カメラが動いても場所についていく
void GameScene::DrawScorePopups()
{
	if (!_camera)
		return;

	const DirectX::XMMATRIX view = _camera->GetViewMatrix();
	const DirectX::XMMATRIX projection = _camera->GetProjectionMatrix();

	for (const auto& popup : _scorePopups)
	{
		const float t = popup.age / popupLifetime;

		const DirectX::XMVECTOR world = DirectX::XMVectorSet(
			popup.worldPosition.x, popup.worldPosition.y, popup.worldPosition.z, 1.0f);
		const DirectX::XMFLOAT2 screen = _uiRenderer->WorldToScreen(world, view, projection);

		const float rise = popupRisePixels * EaseOutCubic(t);
		const float alpha = t < popupFadeStart ? 1.0f : 1.0f - (t - popupFadeStart) / (1.0f - popupFadeStart);

		// 出た瞬間に大きく弾んで、元の大きさに戻る。頭ヒットは、さらに大きくて金色
		const float popT = (std::min)(popup.age / popupPopSeconds, 1.0f);
		const float scale = popupBaseScale * (popup.isHead ? 1.3f : 1.0f) * (1.0f + 0.5f * (1.0f - EaseOutBack(popT)));

		const DirectX::XMVECTOR color = popup.isHead
			? DirectX::XMVectorSet(1.0f, 0.8f, 0.2f, alpha)
			: DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, alpha);

		_uiRenderer->DrawStringCentered(popup.text, screen.x, screen.y - rise, scale,
			color, DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, alpha));
	}
}

// 「COMBO x3」(2コンボ以上のときだけ)
void GameScene::DrawComboText()
{
	const int combo = _scoreManager.GetCombo();
	if (combo < 2)
		return;

	const float pop = 1.0f + 0.5f * (1.0f - EaseOutBack(_comboPop.T()));
	_uiRenderer->DrawStringCentered(L"COMBO x" + std::to_wstring(combo), comboCenterX, comboCenterY,
		comboScale * pop, DirectX::XMVectorSet(1.0f, 0.55f, 0.2f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
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
	constexpr float fontLineHeight = 96.0f;   // UIFont.spritefont の行の高さ(scale 1.0のとき)

	// ラグドールが止まった直後は、1.6倍から弾んで元の大きさに戻る
	const float pop = 1.0f + 0.6f * (1.0f - EaseOutBack(_shotCountPop.T()));

	// 拡大しても数字の中心がずれないように、中心を基準に描く
	const float centerX = panel.frame.left + panel.frame.width * 0.5f;
	const float centerY = valueTop + fontLineHeight * valueScale * 0.5f;
	_uiRenderer->DrawStringCentered(std::to_wstring(_roundManager.GetShotCount()), centerX, centerY,
		valueScale * pop, DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMVectorSet(0, 0, 0, 1), 0.0f);
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