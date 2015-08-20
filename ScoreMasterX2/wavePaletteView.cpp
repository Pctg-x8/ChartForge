#include "wavePaletteView.h"

#include "appContext.h"
#include "colorPalette.h"

const auto ListElementHeight = 20.0f;

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
	this->pInnerView = std::make_unique<InnerView>(this);
	this->pVerticalScrollBar = std::make_unique<VerticalScrollBar>(this);
	this->pBorder = std::make_unique<Border>();

	this->addChild(this->pInnerView.get());
	this->addChild(this->pVerticalScrollBar.get());
	this->addChild(this->pBorder.get());

	this->firstSelection = this->lastSelection = 0xffffffff;
}
WaveListView::~WaveListView() = default;
void WaveListView::updateAll()
{
	this->Layer::updateAll();
	this->pInnerView->updateAll();
	this->pVerticalScrollBar->updateAll();
	this->pBorder->updateAll();
}
void WaveListView::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pInnerView->hitTest(pt)) this->pInnerView->onMouseMove(this->pInnerView->toLocal(pt));
	else if (this->pVerticalScrollBar->hitTest(pt)) this->pVerticalScrollBar->onMouseMove(this->pVerticalScrollBar->toLocal(pt));
	else this->Layer::onMouseMove(pt);
}
void WaveListView::resizeContent(const D2D1_SIZE_F& size)
{
	this->pBorder->resize(size);

	this->pVerticalScrollBar->setOffset(D2D1::Point2F(size.width - ScrollBarThickness, 0.0f));
	this->pVerticalScrollBar->adjustHeight(size.height);
	this->pInnerView->resize(D2D1::SizeF(this->pVerticalScrollBar->getLeft(), size.height));
	this->pVerticalScrollBar->updatePageSize(size.height);
}
std::vector<std::shared_ptr<WaveEntity>> WaveListView::getSelectedObjects()
{
	if (this->firstSelection == 0xffffffff) return std::vector<std::shared_ptr<WaveEntity>>();

	std::vector<std::shared_ptr<WaveEntity>> pw;
	for (auto i = this->firstSelection; i <= this->lastSelection; ++i)
	{
		pw.push_back(getCurrentContext().getProjectManager()->getCurrent()->getWaveEntity(i));
	}
	return pw;
}

void WaveListView::InnerView::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));
	auto pSelectedBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(0x3399ff, 0.25f));

	pRenderContext->clear(ColorPalette::AppBackground);
	
	auto yOffs = -fmodf(static_cast<float>(this->pOuterView->getScrollOffset()), ListElementHeight);
	auto itemOffset = static_cast<size_t>(this->pOuterView->getScrollOffset() / ListElementHeight);
	auto pFormat = getCurrentContext().getRenderDevice()->getStockedFormat(L"uiDefault");
	for (auto iter = getCurrentContext().getProjectManager()->getCurrent()->getIteratorAt_WaveEntities(itemOffset);
	iter != getCurrentContext().getProjectManager()->getCurrent()->getEndIterator_WaveEntities(); ++iter, ++itemOffset)
	{
		if (this->pOuterView->firstSelection <= itemOffset && itemOffset <= this->pOuterView->lastSelection)
		{
			pRenderContext->drawRect(D2D1::RectF(0.0f, yOffs, this->getSize().width, yOffs + ListElementHeight),
				pSelectedBrush.Get());
		}
		pRenderContext->drawString(iter->get()->getFileName(), D2D1::Point2F(2.0f, yOffs), pFormat, pTextBrush.Get());
		yOffs += ListElementHeight;
		if (yOffs >= this->getSize().height) break;
	}
}
void WaveListView::receiveValueChanged(ScrollBarBase* pSender)
{
	getCurrentContext().queueUpdated(this->pInnerView.get());
}
void WaveListView::notifyUpdateEntityList()
{
	this->pVerticalScrollBar->updateFullSize(getCurrentContext().getProjectManager()->getCurrent()->getWaveEntityCount() * ListElementHeight);
	getCurrentContext().queueUpdated(this->pInnerView.get());
}
void WaveListView::InnerView::onMouseMove(const D2D1_POINT_2F& pt)
{
	this->Layer::onMouseMove(pt);

	this->lastPointerPos = pt;
	if (getCurrentContext().isHoldingLayer(this))
	{
		// process additive selection

		if (this->lastPointerPos.y < 0)
		{
			this->pOuterView->pVerticalScrollBar->updateCurrentValue(this->pOuterView->pVerticalScrollBar->getCurrentValue() + this->lastPointerPos.y / 10.0);
		}
		if (this->lastPointerPos.y > this->getSize().height)
		{
			this->pOuterView->pVerticalScrollBar->updateCurrentValue(this->pOuterView->pVerticalScrollBar->getCurrentValue() +
				(this->lastPointerPos.y - this->getSize().height) / 10.0);
		}

		auto yGlobal = this->lastPointerPos.y + this->pOuterView->getScrollOffset();
		auto yIndexGlobal = static_cast<int32_t>(yGlobal / ListElementHeight);
		if (0 <= yIndexGlobal && yIndexGlobal < getCurrentContext().getProjectManager()->getCurrent()->getWaveEntityCount())
		{
			this->pOuterView->lastSelection = yIndexGlobal;
			getCurrentContext().queueUpdated(this);
		}
	}
}
void WaveListView::InnerView::onMouseDown()
{
	getCurrentContext().holdLayerWithEntering(this);

	this->pOuterView->firstSelection = this->pOuterView->lastSelection = 0xffffffff;
	auto yGlobal = this->lastPointerPos.y + this->pOuterView->getScrollOffset();
	auto yIndexGlobal = static_cast<int32_t>(yGlobal / ListElementHeight);
	if (0 <= yIndexGlobal && yIndexGlobal < getCurrentContext().getProjectManager()->getCurrent()->getWaveEntityCount())
	{
		this->pOuterView->firstSelection = this->pOuterView->lastSelection = yIndexGlobal;
		getCurrentContext().queueUpdated(this);
	}
}
void WaveListView::InnerView::onMouseUp()
{
	getCurrentContext().holdLayerWithEntering(nullptr);
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