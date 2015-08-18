#pragma once

#include <windows.h>
#include <string>
#include <wrl.h>
#include "layerManager.h"

using Microsoft::WRL::ComPtr;

class PopupFrame
{
	HWND nativePointer;
	ComPtr<IDCompositionTarget> pTarget;
public:
	PopupFrame(const std::wstring& frameId, WNDPROC pProcCallback);
	~PopupFrame();

	void show();
	void hide();
	void move(const D2D1_POINT_2F& pt);
	inline void showAt(const D2D1_POINT_2F& pt)
	{
		this->move(pt);
		this->show();
	}

	virtual void onShow() {}
	virtual void onHide() {}

	auto getNativePointer() { return this->nativePointer; }
	auto getCompTarget() { return this->pTarget; }
};