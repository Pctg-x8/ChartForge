#include "arrangerView.h"

ArrangerView::ArrangerView() : Layer(D2D1::SizeF(100.0f, 100.0f))
{
	this->pArrangerBorder = std::make_unique<ArrangerBorder>();
	this->pSpaceMask = std::make_unique<ArrangerSpaceMask>();
	this->pSpaceMaskUnder = std::make_unique<ArrangerSpaceMask>();
	this->pArrangerTools = std::make_unique<ArrangerTools>();
	this->pMeasureBar = std::make_unique<MeasureBar>();
	this->pSysTrackView = std::make_unique<SystemTrackView>();
	this->pUserTrackView = std::make_unique<UserTrackView>();

	this->pMeasScrollBar = std::make_unique<HorizontalScrollBar>(this);
	this->pTrackScrollBar = std::make_unique<VerticalScrollBar>(this);

	this->pMeasScrollBar->updatePageProperty(this->pUserTrackView->getSize().width - TracklistWidth, 32 * 4 * BeatWidth);
	this->pTrackScrollBar->updatePageProperty(this->pUserTrackView->getSize().height, 0.0f);

	this->addChild(this->pArrangerTools.get());
	this->addChild(this->pMeasureBar.get());
	this->addChild(this->pSysTrackView.get());
	this->addChild(this->pUserTrackView.get());
	this->addChild(this->pMeasScrollBar.get());
	this->addChild(this->pTrackScrollBar.get());
	this->addChild(this->pSpaceMask.get());
	this->addChild(this->pSpaceMaskUnder.get());
	this->addChild(this->pArrangerBorder.get());
	this->pMeasureBar->setOffset(D2D1::Point2F(TracklistWidth, ToolsSize + ToolSpacing / 2.0f));
	this->pSpaceMask->setOffset(D2D1::Point2F(0.0f, this->pMeasureBar->getTop()));
	this->pSpaceMaskUnder->resize(D2D1::SizeF(TracklistWidth, ScrollBarThickness));
	this->pSysTrackView->setOffset(D2D1::Point2F(0.0f, this->pMeasureBar->getTop() + MeasureHeight));
	this->pUserTrackView->setOffset(D2D1::Point2F(0.0f, this->pSysTrackView->getTop() + TrackHeight * 3.0f + SplitterHeight));
}
ArrangerView::~ArrangerView() = default;

void ArrangerView::resizeContent(const D2D1_SIZE_F& size)
{
	this->getVisual()->SetClip(this->getLocalRect());

	this->pMeasureBar->adjustWidth(size.width - TracklistWidth);
	this->pSysTrackView->resize(D2D1::SizeF(size.width, TrackHeight * 3.0f));
	this->pUserTrackView->resize(D2D1::SizeF(size.width - ScrollBarThickness, size.height - ScrollBarThickness - this->pUserTrackView->getTop()));
	this->pSpaceMaskUnder->setOffset(D2D1::Point2F(0.0f, size.height - ScrollBarThickness));
	this->pArrangerTools->adjustWidth(size.width);
	
	this->pMeasScrollBar->setOffset(D2D1::Point2F(TracklistWidth, size.height - ScrollBarThickness));
	this->pMeasScrollBar->adjustWidth(size.width - ScrollBarThickness - TracklistWidth);
	this->pTrackScrollBar->setOffset(D2D1::Point2F(size.width -  ScrollBarThickness, this->pUserTrackView->getTop()));
	this->pTrackScrollBar->adjustHeight(this->pUserTrackView->getSize().height);

	this->pMeasScrollBar->updatePageSize(this->pUserTrackView->getSize().width - TracklistWidth);
	this->pTrackScrollBar->updatePageSize(this->pUserTrackView->getSize().height);
}
void ArrangerView::updateSystemTracks(SystemTrackView::IteratorT begin_iter, SystemTrackView::IteratorT end_iter)
{
	this->pSysTrackView->updateTrackRange(begin_iter, end_iter);
}
void ArrangerView::updateUserTracks(UserTrackView::IteratorT begin_iter, UserTrackView::IteratorT end_iter)
{
	this->pUserTrackView->updateTrackRange(begin_iter, end_iter);
	this->pTrackScrollBar->updateFullSize(this->pUserTrackView->getInnerHeight());
}
void ArrangerView::updateAll()
{
	this->Layer::updateAll();

	this->pArrangerBorder->updateAll();
	this->pSpaceMask->updateAll();
	this->pArrangerTools->updateAll();
	this->pMeasureBar->updateAll();
	this->pSysTrackView->updateAll();
	this->pUserTrackView->updateAll();
	this->pMeasScrollBar->updateAll();
	this->pTrackScrollBar->updateAll();
}
void ArrangerView::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pArrangerTools->hitTest(pt))
	{
		this->pArrangerTools->onMouseMove(this->pArrangerTools->toLocal(pt));
	}
	else if (this->pMeasScrollBar->hitTest(pt))
	{
		this->pMeasScrollBar->onMouseMove(this->pMeasScrollBar->toLocal(pt));
	}
	else if (this->pTrackScrollBar->hitTest(pt))
	{
		this->pTrackScrollBar->onMouseMove(this->pTrackScrollBar->toLocal(pt));
	}
	else
	{
		this->Layer::onMouseMove(pt);
	}
}
void ArrangerView::receiveValueChanged(ScrollBarBase* pSender)
{
	if (pSender == this->pMeasScrollBar.get())
	{
		getCurrentContext().queueUpdated(this->pMeasureBar.get());
		getCurrentContext().queueUpdated(this->pSysTrackView->getMatrixLayer());
		getCurrentContext().queueUpdated(this->pUserTrackView->getMatrixLayer());
	}
	else if (pSender == this->pTrackScrollBar.get())
	{
		getCurrentContext().queueUpdated(this->pUserTrackView->getTrackLayer());
		getCurrentContext().queueUpdated(this->pUserTrackView->getMatrixLayer());
	}
}

ArrangerBorder::ArrangerBorder() : Layer()
{

}
ArrangerBorder::~ArrangerBorder() = default;
void ArrangerBorder::updateContent(RenderContext* pRenderContext)
{
	auto pBorderBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawRectFrame(this->getLocalRectPixelated(), pBorderBrush.Get());
}

MeasureBar::MeasureBar() : Layer(D2D1::SizeF(100.0f, MeasureHeight))
{

}
MeasureBar::~MeasureBar() = default;

void MeasureBar::adjustWidth(float w)
{
	this->resize(D2D1::SizeF(w, MeasureHeight));
}
void MeasureBar::updateContent(RenderContext* pRenderContext)
{
	auto pLineBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.25f));
	auto pBarBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White));

	pRenderContext->clear(D2D1::ColorF(0x404040));

	auto ch = this->getSize().height;
	auto cf = getCurrentContext().getRenderDevice()->getStockedFormat(L"uiDefault");
	auto beatOffset = static_cast<uint32_t>(floor(getCurrentContext().getArrangerView()->getMeasureScrollOffset() / BeatWidth));
	auto x = -fmodf(static_cast<float>(getCurrentContext().getArrangerView()->getMeasureScrollOffset()), BeatWidth) + 0.5f;
	for (uint32_t beat = beatOffset; ; beat++)
	{
		if (x > this->getSize().width) break;
		if (beat % 4 == 0)
		{
			pRenderContext->drawVerticalLine(x, ch * 0.375f, ch - 0.5f, pBarBrush.Get());
			pRenderContext->drawString(std::to_wstring(beat / 4 + 1), D2D1::Point2F(x + 4.0f, ch * 0.1875f),
				cf, pTextBrush.Get());
		}
		else
		{
			pRenderContext->drawVerticalLine(x, ch * 0.625f, ch - 0.5f, pLineBrush.Get());
		}
		x += BeatWidth;
	}
	pRenderContext->drawHorizontalLine(ch, 0.0f, this->getSize().width, pLineBrush.Get());
}

ArrangerTools::ArrangerTools() : Layer(D2D1::SizeF(100.0f, ToolsSize))
{
	this->pAddTrackCommandButton = std::make_unique<AddTrackCommandButton>();
	this->pRemoveTrackCommandButton = std::make_unique<RemoveTrackCommandButton>();
	this->pQuantizeIndicator = std::make_unique<QuantizeIndicator>();

	this->addChild(this->pAddTrackCommandButton.get());
	this->addChild(this->pRemoveTrackCommandButton.get());
	this->addChild(this->pQuantizeIndicator.get());
	this->pAddTrackCommandButton->setOffset(D2D1::Point2F(ToolSpacing * 2.0f, 0.0f));
	this->pRemoveTrackCommandButton->setOffset(D2D1::Point2F(this->pAddTrackCommandButton->getRight() + ToolSpacing, 0.0f));
	this->pQuantizeIndicator->setOffset(D2D1::Point2F(this->pRemoveTrackCommandButton->getRight() + ToolSpacing * 2.0f, 0.0f));
}
ArrangerTools::~ArrangerTools() = default;
void ArrangerTools::adjustWidth(float w) { this->resize(D2D1::SizeF(w, ToolsSize)); }

void ArrangerTools::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pAddTrackCommandButton->hitTest(pt))
	{
		this->pAddTrackCommandButton->onMouseMove(this->pAddTrackCommandButton->toLocal(pt));
	}
	else if (this->pRemoveTrackCommandButton->hitTest(pt))
	{
		this->pRemoveTrackCommandButton->onMouseMove(this->pRemoveTrackCommandButton->toLocal(pt));
	}
	else if (this->pQuantizeIndicator->hitTest(pt))
	{
		this->pQuantizeIndicator->onMouseMove(this->pQuantizeIndicator->toLocal(pt));
	}
	else
	{
		this->Layer::onMouseMove(pt);
	}
}
void ArrangerTools::updateAll()
{
	this->Layer::updateAll();
	this->pAddTrackCommandButton->updateAll();
	this->pRemoveTrackCommandButton->updateAll();
	this->pQuantizeIndicator->updateAll();
}

ArrangerTools::AddTrackCommandButton::AddTrackCommandButton() : ToolCommandButton()
{

}
ArrangerTools::AddTrackCommandButton::~AddTrackCommandButton() = default;
void ArrangerTools::AddTrackCommandButton::onClick()
{
	getCurrentContext().getProjectManager()->getCurrent()->addEmptyTrack();
	getCurrentContext().getArrangerView()->updateUserTracks(
		getCurrentContext().getProjectManager()->getCurrent()->getBeginIterator_UserTracks(),
		getCurrentContext().getProjectManager()->getCurrent()->getEndIterator_UserTracks());
}

ArrangerTools::RemoveTrackCommandButton::RemoveTrackCommandButton() : ToolCommandButton()
{

}
ArrangerTools::RemoveTrackCommandButton::~RemoveTrackCommandButton() = default;
void ArrangerTools::RemoveTrackCommandButton::onClick()
{

}
ArrangerTools::QuantizeIndicator::QuantizeIndicator() : Layer()
{
	this->pQuantizeDrop = std::make_unique<QuantizeDrop>();

	this->addChild(this->pQuantizeDrop.get());
	this->pQuantizeDrop->setOffset(D2D1::Point2F(getCurrentContext().getRenderDevice()->calcStringWidth(L"Quantize:", L"uiDefault"), 0.0f));
	
	this->resize(D2D1::SizeF(this->pQuantizeDrop->getRight(), ToolsSize));
}
ArrangerTools::QuantizeIndicator::~QuantizeIndicator() = default;
void ArrangerTools::QuantizeIndicator::updateAll()
{
	this->Layer::updateAll();
	this->pQuantizeDrop->updateAll();
}
void ArrangerTools::QuantizeIndicator::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pQuantizeDrop->hitTest(pt))
	{
		this->pQuantizeDrop->onMouseMove(this->pQuantizeDrop->toLocal(pt));
	}
	else
	{
		this->Layer::onMouseMove(pt);
	}
}
void ArrangerTools::QuantizeIndicator::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawString(L"Quantize:", D2D1::Point2F(), L"uiDefault", pTextBrush.Get());
}

ArrangerTools::QuantizeIndicator::QuantizeDrop::QuantizeDrop() : Layer(D2D1::SizeF(100.0f, ToolsSize))
{
	this->pBorderOverlay = std::make_unique<BorderOverlay>();
	this->pArrowOverlay = std::make_unique<ArrowOverlay>();

	this->addChild(this->pBorderOverlay.get());
	this->addChild(this->pArrowOverlay.get());
	this->pBorderOverlay->resize(this->getSize());
	this->pArrowOverlay->setLeft(this->getSize().width - this->pArrowOverlay->getSize().width);
}
ArrangerTools::QuantizeIndicator::QuantizeDrop::~QuantizeDrop() = default;
void ArrangerTools::QuantizeIndicator::QuantizeDrop::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));

	wchar_t str[128] = {};
	if (getCurrentContext().getProjectManager()->getCurrent()->getQuantizeValue() < 1.0f)
	{
		swprintf_s(str, L"1/%d bars", static_cast<int>(getCurrentContext().getProjectManager()->getCurrent()->getQuantizeValueInv()));
	}
	else
	{
		swprintf_s(str, L"%d bars", static_cast<int>(getCurrentContext().getProjectManager()->getCurrent()->getQuantizeValue()));
	}

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawString(str, D2D1::Point2F(), L"uiDefault", pTextBrush.Get());
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::updateAll()
{
	this->Layer::updateAll();
	this->pBorderOverlay->updateAll();
	this->pArrowOverlay->updateAll();
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::onMouseEnter()
{
	this->pBorderOverlay->raiseEffect();
	this->pArrowOverlay->raiseEffect();
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::onMouseLeave()
{
	this->pBorderOverlay->falloffEffect();
	this->pArrowOverlay->falloffEffect();
}

ArrangerTools::QuantizeIndicator::QuantizeDrop::BorderOverlay::BorderOverlay() : Layer()
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->pOpacity->setOpacity(0.0f);
	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
}
ArrangerTools::QuantizeIndicator::QuantizeDrop::BorderOverlay::~BorderOverlay() = default;
void ArrangerTools::QuantizeIndicator::QuantizeDrop::BorderOverlay::updateContent(RenderContext* pRenderContext)
{
	auto pBorderBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawHorizontalLine(this->getSize().height - 4.0f - 0.5f, 0.0f, this->getSize().width, pBorderBrush.Get());
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::BorderOverlay::raiseEffect()
{
	this->pAlphaAnimator->setInverseQuadratic(0.125, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::BorderOverlay::falloffEffect()
{
	this->pAlphaAnimator->setQuadratic(0.125, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}

ArrangerTools::QuantizeIndicator::QuantizeDrop::ArrowOverlay::ArrowOverlay() : Layer()
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pVerticalAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->resize(D2D1::SizeF(getCurrentContext().getRenderDevice()->calcStringWidth(L"\ue011", L"symDefault"), ToolsSize));

	this->pOpacity->setOpacity(0.0f);
	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
}
ArrangerTools::QuantizeIndicator::QuantizeDrop::ArrowOverlay::~ArrowOverlay() = default;
void ArrangerTools::QuantizeIndicator::QuantizeDrop::ArrowOverlay::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawString(L"\ue011", D2D1::Point2F(0.0f, -2.0f), L"symDefault", pTextBrush.Get());
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::ArrowOverlay::raiseEffect()
{
	this->pAlphaAnimator->setInverseQuadratic(0.125, 0.0f, 1.0f);
	this->pVerticalAnimator->setInverseQuadratic(0.125, -8.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->getVisual()->SetOffsetY(this->pVerticalAnimator->getAnimationObject());
	this->pAlphaAnimator->commit();
}
void ArrangerTools::QuantizeIndicator::QuantizeDrop::ArrowOverlay::falloffEffect()
{
	this->pAlphaAnimator->setQuadratic(0.125, 1.0f, 0.0f);
	this->pVerticalAnimator->setQuadratic(0.125, 0.0f, 8.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->getVisual()->SetOffsetY(this->pVerticalAnimator->getAnimationObject());
	this->pAlphaAnimator->commit();
}

ArrangerSpaceMask::ArrangerSpaceMask() : Layer(D2D1::SizeF(TracklistWidth, MeasureHeight))
{
	
}
ArrangerSpaceMask::~ArrangerSpaceMask() = default;
void ArrangerSpaceMask::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(0x404040));
}

SystemTrackView::SystemTrackView() : ArrangerInnerViewBase<Project::SystemTrackListT>()
{

}
SystemTrackView::~SystemTrackView() = default;
void SystemTrackView::exRenderForWaveMatrix(RenderContext* pRenderContext, uint32_t trackIndex, const D2D1_RECT_F& renderArea)
{
	auto pOverlapBrush = pRenderContext->createLinearGradientBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f), D2D1::ColorF(D2D1::ColorF::White, 0.0f));
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.75f));

	switch (trackIndex)
	{
	case 0:
		for (const auto& ti : getCurrentContext().getProjectManager()->getCurrent()->getTempoList())
		{
			wchar_t str[16] = {};
			swprintf_s(str, L"%.2f", ti.tempo);
			auto wlocal = getCurrentContext().getRenderDevice()->calcStringWidth(str, L"uiDefaultBold") * 1.2f;
			auto left_local = ti.pos.barCount * 4.0f * BeatWidth;
			if (ti.pos.beatNum > 0 && ti.pos.beatDenom > 0) left_local += (4.0f * ti.pos.beatNum / ti.pos.beatDenom) * BeatWidth;
			if (renderArea.left + left_local >= renderArea.right) break;
			if (renderArea.left + left_local + wlocal > 0.0f)
			{
				auto tempoChipRect = D2D1::RectF(renderArea.left + left_local, renderArea.top + 2.0f, renderArea.left + left_local + wlocal, renderArea.bottom - 2.0f);
				pOverlapBrush->SetStartPoint(D2D1::Point2F(tempoChipRect.left, tempoChipRect.top));
				pOverlapBrush->SetEndPoint(D2D1::Point2F(tempoChipRect.right, tempoChipRect.top));
				pRenderContext->drawRect(tempoChipRect, pOverlapBrush.Get());
				pRenderContext->drawRectFrame(tempoChipRect, pOverlapBrush.Get());
				pRenderContext->drawString(str, D2D1::Point2F(tempoChipRect.left + 2.0f, tempoChipRect.top + 1.0f), L"uiDefaultBold", pTextBrush.Get());
			}
		}
		break;
	case 1:
		for (const auto& bi : getCurrentContext().getProjectManager()->getCurrent()->getBeatList())
		{
			wchar_t str[64] = {};
			swprintf_s(str, L"%d/%d (x%.2f)", bi.beatNum, bi.beatDenom, static_cast<float>(bi.beatNum) / static_cast<float>(bi.beatDenom));
			auto wlocal = getCurrentContext().getRenderDevice()->calcStringWidth(str, L"uiDefaultBold") * 1.2f;
			auto left_local = bi.barCount * 4.0f * BeatWidth;
			if (renderArea.left + left_local >= renderArea.right) break;
			if (renderArea.left + left_local + wlocal > 0.0f)
			{
				auto beatChipRect = D2D1::RectF(renderArea.left + left_local, renderArea.top + 2.0f, renderArea.left + left_local + wlocal, renderArea.bottom - 2.0f);
				pOverlapBrush->SetStartPoint(D2D1::Point2F(beatChipRect.left, beatChipRect.top));
				pOverlapBrush->SetEndPoint(D2D1::Point2F(beatChipRect.right, beatChipRect.top));
				pRenderContext->drawRect(beatChipRect, pOverlapBrush.Get());
				pRenderContext->drawRectFrame(beatChipRect, pOverlapBrush.Get());
				pRenderContext->drawString(str, D2D1::Point2F(beatChipRect.left + 2.0f, beatChipRect.top + 1.0f), L"uiDefaultBold", pTextBrush.Get());
			}
		}
		break;
	default: break;
	}
}