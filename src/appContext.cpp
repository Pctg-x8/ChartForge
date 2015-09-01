#include "appContext.hpp"

#include <exception>
#include "debugger/debugger.hpp"

// Extra Style //
const auto WS_EX_NOREDIRECTIONBITMAP = 0x00200000L;

// {E22A881C-4005-4A9E-A714-EF614A3895F6}
static const GUID AppID =
{ 0xe22a881c, 0x4005, 0x4a9e,{ 0xa7, 0x14, 0xef, 0x61, 0x4a, 0x38, 0x95, 0xf6 } };

AppContext* getCurrentContext()
{
	static AppContext o;
	return &o;
}

void AppContext::init()
{
	WNDCLASSEX wce = {};
	wchar_t class_name[256] = {0};
	wsprintf(class_name, L"AppContext [ID=%08X-%04X-%04X-%02X%02x-%02x%02x%02x%02x%02x%02x]",
		AppID.Data1, AppID.Data2, AppID.Data3, AppID.Data4[0], AppID.Data4[1],
		AppID.Data4[2], AppID.Data4[3], AppID.Data4[4], AppID.Data4[5], AppID.Data4[6], AppID.Data4[7]);

	wce.cbSize = sizeof wce;
	wce.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
	wce.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wce.hInstance = GetModuleHandle(nullptr);
	wce.lpszClassName = class_name;
	wce.lpfnWndProc = (WNDPROC)&AppContext::WndProc;
	wce.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	if(!RegisterClassEx(&wce)) throw InitializeException(L"RegisterClassEx");

	this->nativePointer = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, wce.lpszClassName, L"AppContext Frame", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wce.hInstance, nullptr);
	if(this->nativePointer == nullptr) throw InitializeException(L"CreateWindowEx");
}

int AppContext::runApplication()
{
	getActiveDebugger()->writeln(L"Starting AppContext...");

	this->init();

	ShowWindow(this->nativePointer, SW_SHOW);

	getActiveDebugger()->writeln(L"msg loop");
	MSG msg;
	while(GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}

LRESULT AppContext::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}