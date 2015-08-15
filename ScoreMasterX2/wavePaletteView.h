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

	std::unique_ptr<VerticalScrollBar> pVerticalScrollBar;
	std::unique_ptr<Border> pBorder;
public:
	WaveListView();
	~WaveListView();
	
	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;

	void receiveValueChanged(ScrollBarBase* pSender) override;
protected:
	void updateContent(RenderContext* pRenderContext) override;
	void resizeContent(const D2D1_SIZE_F& size) override;
};

class WavePaletteView final : public Layer
{
	std::unique_ptr<WaveListView> pWaveListView;
public:
	WavePaletteView();
	~WavePaletteView();

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};