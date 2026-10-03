#include "Title.h"
#include "InputSystem.h"
#include "GameObject.h"
#include "CameraController.h"
#include "UiRenderer.h"
#include "SceneManager.h"

using namespace UiInfo;


void Title::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	BaseScene::Initialize(locator);

	// W/Sはメニュー選択に使うので、タイトルのカメラは動かさない
	for (const auto& object : _objects)
	{
		if (auto cameraController = object->GetComponent<CameraController>())
			cameraController->SetMoveEnabled(false);
	}
}

void Title::Update(float deltaTime)
{
	if (_input->GetKeyDown('W') || _input->GetKeyDown(VK_UP))
		_selectedIndex = (_selectedIndex + MenuCount - 1) % MenuCount;

	if (_input->GetKeyDown('S') || _input->GetKeyDown(VK_DOWN))
		_selectedIndex = (_selectedIndex + 1) % MenuCount;

	else if (_input->GetKeyDown(VK_SPACE))
	{
		ConfirmSelected();
	}

	if (_input->GetKeyDown(VK_RETURN))
		_currentPanel = PanelType::None;

	BaseScene::Update(deltaTime);
}


void Title::ConfirmSelected()
{
	switch (MenuEntries[_selectedIndex].text)
	{
	case TextType::Start:
		_sceneManager->ChangeScene(SceneName::GameScene);
		break;
	case TextType::Setting:
		_currentPanel = PanelType::Setting;
		break;
	default:
		break;
	}
}

void Title::DrawUi()
{
	_uiRenderer->Begin();
	if (_currentPanel == PanelType::None)
	{
		for (int i = 0;i < MenuCount;i++)
			DrawMenuItem(MenuEntries[i], i);
	}

	if (_currentPanel != PanelType::None)
		DrawPanel(_currentPanel);

	_uiRenderer->DrawString(L"ロケットにんげん", 380.0f, 100.0f, 1.0f,
		DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
	_uiRenderer->End();
}

void Title::DrawMenuItem(MenuEntry menu,int index)
{
	auto buttonIterator = buttonList.find(menu.button);
	auto textIterator = textList.find(menu.text);
	if (buttonIterator == buttonList.end() || textIterator == textList.end()) return;

	const auto& buttonData = buttonIterator->second;
	const auto& textData = textIterator->second;;

	_uiRenderer->DrawFrame(buttonData.left, buttonData.top, buttonData.width, buttonData.height,
		buttonData.thickness,
		index == _selectedIndex ? XMVectorSet(1, 1, 0, 1) : XMLoadFloat4(&buttonData.frameColor));

	_uiRenderer->DrawStringCentered(textData.text, textData.centerX, textData.centerY, textData.scale,
		XMLoadFloat4(&textData.mainColor), XMLoadFloat4(&textData.outlineColor));
}

void Title::DrawPanel(PanelType typeIndex)
{
	auto it = panelList.find(typeIndex);
	if (it == panelList.end()) return;
	const PanelData& panel = it->second;

	_uiRenderer->DrawRect(panel.frame.left, panel.frame.top, panel.frame.width, panel.frame.height,
		XMLoadFloat4(&panel.fillColor));
	_uiRenderer->DrawFrame(panel.frame.left, panel.frame.top, panel.frame.width, panel.frame.height,
		panel.frame.thickness, XMLoadFloat4(&panel.frame.frameColor));

	for (int i = 0; i < static_cast<int>(panel.items.size()); i++)
		DrawMenuItem(panel.items[i], i);
}