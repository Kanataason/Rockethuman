#include "Title.h"
#include "InputSystem.h"
#include "UiRenderer.h"
#include "SceneManager.h"

using namespace UiInfo;

void Title::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	_renderer = locator.uiRenderer;
	BaseScene::Initialize(locator);
}	

void Title::Update(float deltaTime)
{
	UpdateStartButton(_input->GetMouseClientPosition());

	if (_isStartButtonHovered && _input->GetKeyDown(VK_LBUTTON))
	{
		_sceneManager->ChangeScene(SceneName::GameScene);
		//押したときの処理
	}
	BaseScene::Update(deltaTime);
}
void Title::UpdateStartButton(const POINT& mousePos)
{
	auto buttonIterator = buttonList.find(ButtonType::Box);
	if (buttonIterator == buttonList.end()) return;

	const auto& buttonData = buttonIterator->second;
	_isStartButtonHovered = _renderer->HitTest(buttonData.left, buttonData.top,
		buttonData.width, buttonData.height, mousePos);

}

void Title::DrawUi()
{
	_renderer->Begin();
	DrawStartButton();
	_renderer->DrawString(L"あいうえお", 500.0f, 100.0f, 1.0f,
		DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
	_renderer->End();
}

void Title::DrawStartButton()
{
	auto buttonIterator = buttonList.find(ButtonType::Box);
	auto textIterator = textList.find(TextType::Start);
	if (buttonIterator == buttonList.end() || textIterator == textList.end()) return;

	const auto& buttonData = buttonIterator->second;
	const auto& textData = textIterator->second;

	_renderer->DrawFrame(buttonData.left, buttonData.top, buttonData.width, buttonData.height,
		buttonData.thickness,
		_isStartButtonHovered ? XMVectorSet(1, 1, 0, 1) : XMLoadFloat4(&buttonData.frameColor));

	_renderer->DrawStringCentered(textData.text, textData.centerX, textData.centerY, textData.scale,
		XMLoadFloat4(&textData.mainColor), XMLoadFloat4(&textData.outlineColor));
}