#include "appContext.h"

#include <cstdio>
#include <exception>
#include <string>
#include <cassert>
#include <mfapi.h>

#include "arrangerView.h"
#include "backgroundLayer.h"

using namespace std::string_literals;

// {E22A881C-4005-4A9E-A714-EF614A3895F6}
static const GUID AppID =
{ 0xe22a881c, 0x4005, 0x4a9e,{ 0xa7, 0x14, 0xef, 0x61, 0x4a, 0x38, 0x95, 0xf6 } };

AppContext& getCurrentContext()
{
	static AppContext o;
	return o;
}

AppContext::AppContext() = default;
AppContext::~AppContext() = default;

int AppContext::runApplication()
{
	ComResult hr;

	this->initNativePointer();
	hr = OleInitialize(nullptr);

	this->queuedUpdated = false;
	this->pRenderDevice = std::make_unique<RenderDevice>();
	this->pRenderDevice->init();
	this->pLayerManager = std::make_unique<LayerManager>();
	this->pLayerManager->init<BackgroundLayer>();
	this->pProjectManager = std::make_unique<ProjectManager>();

	this->pRenderDevice->stockFormat(L"uiDefault", L"Yu Gothic UI", 9.0f);
	this->pRenderDevice->stockFormat(L"uiDefaultBold", L"Yu Gothic UI", 9.0f, DWRITE_FONT_WEIGHT_BOLD);
	this->pRenderDevice->stockFormat(L"uiDefaultBig", L"Yu Gothic UI", 60.0f);
	this->pRenderDevice->stockFormat(L"symDefault", L"Segoe UI Symbol", 9.5f);
	this->pScreenSelector = std::make_unique<ScreenSelector>();
	this->pScreenSelector->setOffset(D2D1::Point2F(0.0f, 0.0f));
	this->pLayerManager->getRootLayer()->addChild(this->pScreenSelector.get());
	this->pArrangerViewHolder = std::make_unique<ScreenHolder<ArrangerPage>>();
	this->pChartEditorHolder = std::make_unique<ScreenHolder<ChartEditor>>();
	this->pArrangerViewHolder->setInitialState(true);
	this->pChartEditorHolder->setInitialState(false);
	this->pLayerManager->getRootLayer()->addChild(this->pArrangerViewHolder.get());
	this->pLayerManager->getRootLayer()->addChild(this->pChartEditorHolder.get());

	this->pQuantizeSelectorPopup = std::make_unique<QuantizeSelectorPopup>();

	this->pSoundLoaderThread = std::make_unique<SoundLoader>();
	this->pLayerManager->getRootLayer()->addChild(this->pSoundLoaderThread->getOverlay());

	{
		ComPtr<IDropTarget> pDropTarget;
		hr = CDropTarget::createInstance(&pDropTarget);
		hr = RegisterDragDrop(this->nativePointer, pDropTarget.Get());
	}
	this->pDragScreenOverlay = std::make_unique<DragScreenOverlay>();
	this->pLayerManager->getRootLayer()->addChild(this->pDragScreenOverlay.get());

	this->pArrangerViewHolder->getContent()->getLeftView()->updateSystemTracks(
		this->pProjectManager->getCurrent()->getBeginIterator_SystemTracks(),
		this->pProjectManager->getCurrent()->getEndIterator_SystemTracks());

	this->pLayerManager->getRootLayer()->updateAll();
	this->pScreenSelector->updateAll();

	this->pWaitChipFrame = std::make_unique<ExitWaitChip>();

	auto windowName = L"ChartForge - "s;
	windowName.append(this->pProjectManager->getFilePath());
	SetWindowText(this->nativePointer, windowName.c_str());
	ShowWindow(this->nativePointer, SW_SHOW);
	MSG msg;
	while (true)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				return static_cast<int>(msg.wParam);
			}
		}

		this->processUpdates();
		Sleep(1);
	}
}

void AppContext::initNativePointer()
{
	// make class name
	wchar_t wsClassName[256] = { 0 };
	swprintf_s(wsClassName, L"AppContext.NativePointer [AppID=%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x]",
		AppID.Data1, AppID.Data2, AppID.Data3, AppID.Data4[0], AppID.Data4[1],
		AppID.Data4[2], AppID.Data4[3], AppID.Data4[4], AppID.Data4[5], AppID.Data4[6], AppID.Data4[7]);

	WNDCLASSEX wce = {};
	wce.cbSize = sizeof wce;
	wce.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
	wce.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wce.hInstance = GetModuleHandle(nullptr);
	wce.lpszClassName = wsClassName;
	wce.lpfnWndProc = AppContext::WndProc;
	wce.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	if (RegisterClassEx(&wce) == 0) throw std::exception("RegisterClassEx failed");

	this->nativePointer = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, wsClassName, L"AppContext Frame", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wce.hInstance, nullptr);
	if (this->nativePointer == nullptr) throw std::exception("CreateWindowEx failed");
}

LRESULT CALLBACK AppContext::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		getCurrentContext().pWaitChipFrame->show();
		getCurrentContext().pSoundLoaderThread.reset();
		OleUninitialize();
		RevokeDragDrop(hWnd);
		getCurrentContext().pWaitChipFrame->hide();
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		getCurrentContext().getLayerManager()->getRootLayer()->resize(D2D1::SizeF(LOWORD(lParam), HIWORD(lParam)));
		getCurrentContext().pScreenSelector->adjustWidth(LOWORD(lParam));
		getCurrentContext().pArrangerViewHolder->resize(D2D1::SizeF(LOWORD(lParam), HIWORD(lParam) - getCurrentContext().pScreenSelector->getBottom()));
		getCurrentContext().pChartEditorHolder->resize(D2D1::SizeF(LOWORD(lParam), HIWORD(lParam) - getCurrentContext().pScreenSelector->getBottom()));
		getCurrentContext().pArrangerViewHolder->setTop(getCurrentContext().pScreenSelector->getBottom());
		getCurrentContext().pChartEditorHolder->setTop(getCurrentContext().pScreenSelector->getBottom());
		getCurrentContext().pDragScreenOverlay->resize(D2D1::SizeF(LOWORD(lParam), HIWORD(lParam)));
		getCurrentContext().pSoundLoaderThread->getOverlay()->resize(D2D1::SizeF(LOWORD(lParam), HIWORD(lParam)));
		// Force Update
		getCurrentContext().processUpdates();
		break;
	case WM_MOUSEMOVE:
		return getCurrentContext().onMouseMove(wParam, lParam);
	case WM_LBUTTONDOWN:
		return getCurrentContext().onMouseDown(wParam, lParam);
	case WM_LBUTTONUP:
		return getCurrentContext().onMouseUp(wParam, lParam);
	case WM_MOUSELEAVE:
		getCurrentContext().notifyEnteredLayer(nullptr);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void AppContext::processMessagesForWaitFrame()
{
	MSG msg;
	while (PeekMessage(&msg, this->pWaitChipFrame->getNativePointer(), 0, 0, PM_REMOVE) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void AppContext::queueUpdated(Layer* pLayer)
{
	this->updatedLayers.push_back(pLayer);
	this->queuedUpdated = true;
}
void AppContext::processUpdates()
{
	if (this->queuedUpdated)
	{
		// Update Layers...
		for (auto pl : this->updatedLayers)
		{
			// layer updating
			pl->updateEntry();
		}
		this->pLayerManager->getDevice()->Commit();
		this->updatedLayers.clear();
		this->queuedUpdated = false;
	}
}
void AppContext::notifyEnteredLayer(Layer* pLayer)
{
	if (this->pEnterSigHoldingLayer != nullptr) return;
	if (this->pEnteredLayer == pLayer) return;

	if (this->pEnteredLayer != nullptr) this->pEnteredLayer->onMouseLeave();
	this->pEnteredLayer = pLayer;
	if (this->pEnteredLayer != nullptr) this->pEnteredLayer->onMouseEnter();
}
void AppContext::holdLayer(Layer* pLayer)
{
	this->pHoldingLayer = pLayer;
	if (this->pHoldingLayer != nullptr) SetCapture(this->nativePointer);
	else ReleaseCapture();
}
void AppContext::holdLayerWithEntering(Layer* pLayer)
{
	this->holdLayer(pLayer);
	this->pEnterSigHoldingLayer = pLayer;
	if (this->pEnterSigHoldingLayer != nullptr && this->pEnterSigHoldingLayer != this->pEnteredLayer)
	{
		this->notifyEnteredLayer(this->pEnterSigHoldingLayer);
	}
}
LRESULT AppContext::onMouseMove(WPARAM wParam, LPARAM lParam)
{
	auto pCursor = D2D1::Point2F(static_cast<float>(LOWORD(lParam)), static_cast<float>(HIWORD(lParam)));
	if (this->pSoundLoaderThread->getOverlay()->isEffectRaised())
	{
		this->notifyEnteredLayer(nullptr);
		return 0;
	}
	else if (this->pEnterSigHoldingLayer != nullptr)
	{
		this->pEnterSigHoldingLayer->onMouseMove(this->pEnterSigHoldingLayer->fromGlobal(pCursor));
		return 0;
	}
	else if (this->pHoldingLayer == nullptr)
	{
		TRACKMOUSEEVENT tme = {};
		tme.cbSize = sizeof tme;
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = this->nativePointer;
		TrackMouseEvent(&tme);

		if (this->pDragScreenOverlay->isEffectRaised())
		{
			this->notifyEnteredLayer(nullptr);
		}
		else
		{
			if (this->pScreenSelector->hitTest(pCursor))
			{
				this->pScreenSelector->onMouseMove(this->pScreenSelector->toLocal(pCursor));
			}
			else if (this->pArrangerViewHolder->isShown() && this->pArrangerViewHolder->hitTest(pCursor))
			{
				this->pArrangerViewHolder->onMouseMove(this->pArrangerViewHolder->toLocal(pCursor));
			}
			else
			{
				this->notifyEnteredLayer(nullptr);
			}
		}
		return 0;
	}
	else
	{
		if (this->pHoldingLayer->hasParent())
		{
			if (this->pHoldingLayer->hitTest(this->pHoldingLayer->getParent()->fromGlobal(pCursor)))
			{
				this->pHoldingLayer->onMouseMove(this->pHoldingLayer->fromGlobal(pCursor));
			}
			else this->notifyEnteredLayer(nullptr);
		}
		else
		{
			if (this->pHoldingLayer->hitTest(pCursor))
			{
				this->pHoldingLayer->onMouseMove(this->pHoldingLayer->toLocal(pCursor));
			}
			else this->notifyEnteredLayer(nullptr);
		}
		return 0;
	}
}
LRESULT AppContext::onMouseDown(WPARAM wParam, LPARAM lParam)
{
	this->onMouseMove(wParam, lParam);
	if (this->pHoldingLayer == nullptr)
	{
		if (this->pEnteredLayer != nullptr) this->pEnteredLayer->onMouseDown();
	}
	else
	{
		this->pHoldingLayer->onMouseDown();
	}
	return 0;
}
LRESULT AppContext::onMouseUp(WPARAM wParam, LPARAM lParam)
{
	this->onMouseMove(wParam, lParam);
	if (this->pHoldingLayer == nullptr)
	{
		if (this->pEnteredLayer != nullptr) this->pEnteredLayer->onMouseUp();
	}
	else
	{
		this->pHoldingLayer->onMouseUp();
	}
	return 0;
}
void AppContext::changeScreen(SelectorIds sid)
{
	switch (sid)
	{
	case SelectorIds::ArrangerView:
		this->pArrangerViewHolder->show();
		this->pChartEditorHolder->hide();
		break;
	case SelectorIds::ChartEditor:
		this->pArrangerViewHolder->hide();
		this->pChartEditorHolder->show();
		break;
	default: assert(false);
	}
}
void AppContext::changeQuantizeValue(double v)
{
	this->pProjectManager->getCurrent()->setQuantizeValueInv(v);
	this->pArrangerViewHolder->getContent()->getLeftView()->notifyUpdateQuantizeValue();
}