#include "popupFrame.h"
#include "appContext.h"
#include "comutils.h"

#include <string>
using namespace std::string_literals;

PopupFrame::PopupFrame(const std::wstring& frameID, WNDPROC pProcCallback)
{
	wchar_t frameClassName[256] = {};
	swprintf_s(frameClassName, L"AppPopupFrame [ID=%s]", frameID.c_str());

	WNDCLASSEX wce = { 0 };
	wce.cbSize = sizeof wce;
	wce.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wce.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wce.lpszClassName = frameClassName;
	wce.hInstance = GetModuleHandle(nullptr);
	wce.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wce.lpfnWndProc = pProcCallback;
	if (!RegisterClassEx(&wce)) throw std::exception("PopupFrame Registering failed");

	this->nativePointer = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, wce.lpszClassName, L"AppPopupFrame",
		WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr,
		nullptr, wce.hInstance, nullptr);
	if (this->nativePointer == nullptr) throw std::exception("PopupFrame Native Pointer create failed");

	this->pTarget = getCurrentContext().getLayerManager()->createTarget(this->nativePointer);
}
PopupFrame::~PopupFrame()
{
	DestroyWindow(this->nativePointer);
	this->nativePointer = nullptr;
}

void PopupFrame::show()
{
	ShowWindow(this->nativePointer, SW_SHOW);
	this->onShow();
}
void PopupFrame::hide()
{
	ShowWindow(this->nativePointer, SW_HIDE);
	this->onHide();
}

void PopupFrame::move(const D2D1_POINT_2F& pt)
{
	SetWindowPos(this->nativePointer, nullptr, static_cast<int>(pt.x), static_cast<int>(pt.y), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}