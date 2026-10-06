#include "MenuList.h"

#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include "InputSystem.h"
#include "UIRenderer.h"

using namespace DirectX;

void MenuList::Initialize(ServiceLocator& locator)
{
	_input = locator.input;
	_uiRenderer = locator.uiRenderer;
}

void MenuList::SetItems(std::vector<std::wstring> items)
{
	_items = std::move(items);
	SetSelectedIndex(_selectedIndex);
}

void MenuList::SetSelectedIndex(int index)
{
	_selectedIndex = std::clamp(index, 0, (std::max)(0, GetCount() - 1));
	AdjustScroll();
}

void MenuList::Update()
{
	const int count = GetCount();
	if (!_input || count <= 1)
		return;

	int next = _selectedIndex;
	if (_input->GetKeyDown('W') || _input->GetKeyDown(VK_UP))
		next = (next + count - 1) % count;
	if (_input->GetKeyDown('S') || _input->GetKeyDown(VK_DOWN))
		next = (next + 1) % count;

	if (next == _selectedIndex)
		return;

	_selectedIndex = next;
	AdjustScroll();
	OnMoved.Invoke(_selectedIndex);
}

void MenuList::AdjustScroll()
{
	const int count = GetCount();

	if (_selectedIndex < _scroll)
		_scroll = _selectedIndex;
	if (_selectedIndex >= _scroll + _layout.visibleRows)
		_scroll = _selectedIndex - _layout.visibleRows + 1;

	_scroll = std::clamp(_scroll, 0, (std::max)(0, count - _layout.visibleRows));
}

void MenuList::Draw()
{
	if (!_uiRenderer)
		return;

	const int count = GetCount();
	if (count == 0)
	{
		_uiRenderer->DrawStringCentered(_emptyMessage, 640.0f, _layout.emptyY, 0.45f,
			XMVectorSet(1, 1, 1, 1), XMVectorSet(0, 0, 0, 1));
		return;
	}

	const int rows = (std::min)(count, _layout.visibleRows);
	for (int row = 0; row < rows; row++)
	{
		const int index = _scroll + row;
		const float top = _layout.top + (_layout.height + _layout.gap) * static_cast<float>(row);
		const bool selected = (index == _selectedIndex);

		_uiRenderer->DrawFrame(_layout.left, top, _layout.width, _layout.height, 2.0f,
			selected ? XMVectorSet(1, 1, 0, 1) : XMVectorSet(1, 1, 1, 1));
		_uiRenderer->DrawStringCentered(_items[static_cast<size_t>(index)],
			_layout.left + _layout.width * 0.5f, top + _layout.height * 0.5f, _layout.textScale,
			selected ? XMVectorSet(1, 1, 0, 1) : XMVectorSet(1, 1, 1, 1),
			XMVectorSet(0, 0, 0, 1));
	}

	//見えていない行があるときだけ、いま何番目かを出す
	if (count > _layout.visibleRows)
	{
		_uiRenderer->DrawStringCentered(std::to_wstring(_selectedIndex + 1) + L" / " + std::to_wstring(count),
			640.0f, _layout.counterY, 0.3f, XMVectorSet(0.7f, 0.8f, 1.0f, 1), XMVectorSet(0, 0, 0, 1));
	}
}
