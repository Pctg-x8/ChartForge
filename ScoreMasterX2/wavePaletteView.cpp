#include "wavePaletteView.h"

#include "colorPalette.h"

WavePaletteView::WavePaletteView() : Layer()
{
	this->pWaveListView = std::make_unique<WaveListView>();

	this->pWaveListView->setOffset(D2D1::Point2F(4.0f, 4.0f));
	this->addChild(this->pWaveListView.get());
}
WavePaletteView::~WavePaletteView() = default;
void WavePaletteView::updateAll()
{
	this->Layer::updateAll();
	this->pWaveListView->updateAll();
}
void WavePaletteView::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pWaveListView->hitTest(pt))
	{
		this->pWaveListView->onMouseMove(this->pWaveListView->toLocal(pt));
	}
	else
	{
		this->Layer::onMouseMove(pt);
	}
}
void WavePaletteView::resizeContent(const D2D1_SIZE_F& size)
{
	this->pWaveListView->resize(D2D1::SizeF(size.width - 8.0f, size.height - 4.0f - this->pWaveListView->getTop()));
}

WaveListView::WaveListView() : Layer()
{
	this->pVerticalScrollBar = std::make_unique<VerticalScrollBar>(this);
	this->pBorder = std::make_unique<Border>();

	this->addChild(this->pVerticalScrollBar.get());
	this->addChild(this->pBorder.get());
}
WaveListView::~WaveListView() = default;
void WaveListView::updateAll()
{
	this->Layer::updateAll();
	this->pBorder->updateAll();
}
void WaveListView::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pVerticalScrollBar->hitTest(pt)) this->pVerticalScrollBar->onMouseMove(this->pVerticalScrollBar->toLocal(pt));
	else this->Layer::onMouseMove(pt);
}
void WaveListView::resizeContent(const D2D1_SIZE_F& size)
{
	this->pBorder->resize(size);

	this->pVerticalScrollBar->setOffset(D2D1::Point2F(size.width - ScrollBarThickness, 0.0f));
	this->pVerticalScrollBar->adjustHeight(size.height);
}
void WaveListView::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(ColorPalette::AppBackground);
}
void WaveListView::receiveValueChanged(ScrollBarBase* pSender)
{

}

WaveListView::Border::Border() : Layer()
{

}
WaveListView::Border::~Border() = default;
void WaveListView::Border::updateContent(RenderContext* pRenderContext)
{
	auto pBorderBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.25f));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawRectFrame(this->getLocalRectPixelated(), pBorderBrush.Get());
}