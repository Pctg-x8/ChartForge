#pragma once

#include <windows.h>
#include <memory>
#include <wrl.h>
#include <list>

#include "RenderDevice.h"
#include "layerManager.h"

#include "ddManager.h"
#include "project.h"

#include "screenSelector.h"
#include "screenHolder.h"
#include "arrangerPage.h"
#include "chartEditor.h"
#include "dragScreenOverlay.h"

#include "quantizeSelectorPopup.h"

using Microsoft::WRL::ComPtr;

class AppContext
{
	friend AppContext& getCurrentContext();

	AppContext();
	~AppContext();

	HWND nativePointer;
	std::unique_ptr<RenderDevice> pRenderDevice;
	std::unique_ptr<LayerManager> pLayerManager;

	std::unique_ptr<ProjectManager> pProjectManager;

	std::unique_ptr<ScreenSelector> pScreenSelector;
	std::unique_ptr<ScreenHolder<ArrangerPage>> pArrangerViewHolder;
	std::unique_ptr<ScreenHolder<ChartEditor>> pChartEditorHolder;
	std::unique_ptr<DragScreenOverlay> pDragScreenOverlay;

	std::unique_ptr<QuantizeSelectorPopup> pQuantizeSelectorPopup;

	bool queuedUpdated;
	std::list<Layer*> updatedLayers;
	Layer* pEnteredLayer = nullptr;
	Layer* pHoldingLayer = nullptr;
	Layer* pEnterSigHoldingLayer = nullptr;

	void initNativePointer();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	void processUpdates();
	LRESULT onMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT onMouseDown(WPARAM wParam, LPARAM lParam);
	LRESULT onMouseUp(WPARAM wParam, LPARAM lParam);
public:
	int runApplication();

	auto getNativePointer() { return this->nativePointer; }
	auto getRenderDevice() { return this->pRenderDevice.get(); }
	auto getLayerManager() { return this->pLayerManager.get(); }
	auto getProjectManager() { return this->pProjectManager.get(); }

	auto getScreenSelector() { return this->pScreenSelector.get(); }
	auto getArrangerView() { return this->pArrangerViewHolder->getContent()->getLeftView(); }
	auto getChartEditor() { return this->pChartEditorHolder->getContent(); }
	auto getDragScreenOverlay() { return this->pDragScreenOverlay.get(); }

	auto getQuantizeSelectorPopup() { return this->pQuantizeSelectorPopup.get(); }

	auto isEnteringLayer(Layer* p) { return this->pEnteredLayer == p; }
	auto isHoldingLayer(Layer* p) { return this->pHoldingLayer == p; }

	void queueUpdated(Layer* pLayer);
	void notifyEnteredLayer(Layer* pLayer);
	void holdLayer(Layer* pLayer);
	void holdLayerWithEntering(Layer* pLayer);

	void changeScreen(SelectorIds sid);
	void changeQuantizeValue(double v);
	inline auto toDesktopPos(const D2D1_POINT_2F& pt)
	{
		POINT p;
		p.x = static_cast<LONG>(pt.x); p.y = static_cast<LONG>(pt.y);
		MapWindowPoints(this->nativePointer, nullptr, &p, 1);
		return D2D1::Point2F(static_cast<float>(p.x), static_cast<float>(p.y));
	}
};

AppContext& getCurrentContext();