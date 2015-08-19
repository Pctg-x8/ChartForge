#pragma once

#include "layerManager.h"
#include "scrollBars.h"

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
	public:
		InnerView() : Layer() {}
		~InnerView() = default;
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	std::unique_ptr<InnerView> pInnerView;
	std::unique_ptr<VerticalScrollBar> pVerticalScrollBar;
	std::unique_ptr<Border> pBorder;
public:
	WaveListView();
	~WaveListView();
	
	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;

	auto getInnerView() { return this->pInnerView.get(); }

	void receiveValueChanged(ScrollBarBase* pSender) override;
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