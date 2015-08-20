#pragma once

#include "layerManager.h"
#include "scrollBars.h"

class WaveEntity;
class WaveListView final : public Layer, public IScrollBarValueReceptor
{
	class Border final : public Layer
	{
	public:
		Border();
		~Border();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};
	class InnerView final : public Layer
	{
		WaveListView* pOuterView;

		D2D1_POINT_2F lastPointerPos;
	public:
		InnerView(WaveListView* pOuter) : Layer() { this->pOuterView = pOuter; }
		~InnerView() = default;

		void onMouseMove(const D2D1_POINT_2F& pt) override;
		void onMouseDown() override;
		void onMouseUp() override;
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	size_t firstSelection, lastSelection;

	std::unique_ptr<InnerView> pInnerView;
	std::unique_ptr<VerticalScrollBar> pVerticalScrollBar;
	std::unique_ptr<Border> pBorder;
public:
	WaveListView();
	~WaveListView();
	
	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;

	auto getScrollOffset() { return this->pVerticalScrollBar->getCurrentValue(); }
	std::vector<std::shared_ptr<WaveEntity>> getSelectedObjects();

	void receiveValueChanged(ScrollBarBase* pSender) override;
	void notifyUpdateEntityList();
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};

class WavePaletteView final : public Layer
{
	std::unique_ptr<WaveListView> pWaveListView;
public:
	WavePaletteView();
	~WavePaletteView();

	auto getWaveListView() { return this->pWaveListView.get(); }

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};