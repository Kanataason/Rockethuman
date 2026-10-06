#pragma once
#include <string>
#include <vector>

#include "Event.h"
#include "ServiceLocator.h"

class InputSystem;
class UiRenderer;

//縦に並んだ選択肢。W/Sで選んで、黄色の枠で、いま選んでいる行を示す
class MenuList
{
public:
	struct Layout
	{
		float left = 490.0f; //1280x720 基準
		float top = 225.0f;
		float width = 300.0f;
		float height = 56.0f;
		float gap = 10.0f; //行と行の間
		float textScale = 0.55f;
		int visibleRows = 4; //一度に見える行の数
		float counterY = 495.0f; //「2 / 5」を出す高さ(見えない行があるときだけ出す)
		float emptyY = 330.0f; //項目が空のときの、メッセージの高さ
	};

	//W/S で、選んでいる行が変わったとき(引数は、新しい番号)
	Event<int> OnMoved;

	void Initialize(ServiceLocator& locator);

	void SetLayout(const Layout& layout) { _layout = layout; }
	void SetEmptyMessage(const std::wstring& message) { _emptyMessage = message; }

	//項目を入れ替える。選んでいる番号は、範囲の中に直す
	void SetItems(std::vector<std::wstring> items);
	void SetSelectedIndex(int index);

	int GetSelectedIndex() const { return _selectedIndex; }
	int GetCount() const { return static_cast<int>(_items.size()); }

	//W/S の入力を受け付ける
	void Update();
	void Draw();

private:
	//選んでいる行が、見えている行の中に入るように、スクロールの位置を直す
	void AdjustScroll();

private:
	InputSystem* _input = nullptr;
	UiRenderer* _uiRenderer = nullptr;

	Layout _layout;
	std::wstring _emptyMessage;

	std::vector<std::wstring> _items;
	int _selectedIndex = 0;
	int _scroll = 0; //いちばん上に見えている項目の番号
};
