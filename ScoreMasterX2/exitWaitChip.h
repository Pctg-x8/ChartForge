#pragma once

#include "popupFrame.h"

class ExitWaitChip : public PopupFrame
{
	ComPtr<IDCompositionVisual2> pVisual;
	ComPtr<IDCompositionSurface> pSurface;

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
public:
	ExitWaitChip();
	~ExitWaitChip() = default;
};