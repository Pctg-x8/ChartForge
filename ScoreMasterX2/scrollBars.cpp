#include "scrollBars.h"
#include "colorPalette.h"
#include "appContext.h"

const auto ThumbMargin = 3.0f;

ScrollBarBase::ScrollBarBase(IScrollBarValueReceptor* pReceptor) : Layer()
{
	this->pReceptor = pReceptor;

	this->pageSize = 50.0f;
	this->fullSize = 100.0f;
	this->currentValue = 0.0f;

	this->pThumb = std::make_unique<ScrollingThumb>(this);

	this->ScrollBarBase::validate();
}
ScrollBarBase::~ScrollBarBase() = default;
void ScrollBarBase::updateAll()
{
	this->Layer::updateAll();
	this->pThumb->updateAll();
}
void ScrollBarBase::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->isValid() && this->pThumb->hitTest(pt))
	{
		this->pThumb->onMouseMove(this->pThumb->toLocal(pt));
	}
	else
	{
		this->Layer::onMouseMove(pt);
	}
}
void ScrollBarBase::updateCurrentValue(double v)
{
	if (this->fullSize - this->pageSize < v) v = this->fullSize - this->pageSize;
	if (v < 0) v = 0;
	if (v != this->currentValue)
	{
		this->currentValue = v;
		if (this->pReceptor != nullptr) this->pReceptor->receiveValueChanged(this);
		this->resizeContent();
	}
}
void ScrollBarBase::updatePageProperty(double p, double f)
{
	this->pageSize = p;
	this->fullSize = f;
	this->updateValidStatus();
	this->updateCurrentValue(this->currentValue);
	this->resizeContent();
}
void ScrollBarBase::updatePageSize(double p)
{
	this->pageSize = p;
	this->updateValidStatus();
	this->updateCurrentValue(this->currentValue);
	this->resizeContent();
}
void ScrollBarBase::updateFullSize(double f)
{
	this->fullSize = f;
	this->updateValidStatus();
	this->updateCurrentValue(this->currentValue);
	this->resizeContent();
}
void ScrollBarBase::validate()
{
	this->is_valid = true;
	this->addChild(this->pThumb.get());
}
void ScrollBarBase::invalidate()
{
	this->is_valid = false;
	this->removeChild(this->pThumb.get());
}
void ScrollBarBase::updateValidStatus()
{
	if (this->fullSize <= 0 || this->pageSize <= 0 || this->fullSize <= this->pageSize)
	{
		if (this->isValid()) this->invalidate();
	}
	else
	{
		if (!this->isValid()) this->validate();
	}
}
void ScrollBarBase::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::Black, 0.125f));
}

VerticalScrollBar::VerticalScrollBar(IScrollBarValueReceptor* pReceptor) : ScrollBarBase(pReceptor)
{
	this->pNegativeButton = std::make_unique<ScrollUpButton>();
	this->pPositiveButton = std::make_unique<ScrollDownButton>();

	this->addChild(this->pNegativeButton.get());
	this->addChild(this->pPositiveButton.get());
}
VerticalScrollBar::~VerticalScrollBar() = default;
void VerticalScrollBar::adjustHeight(float v) { this->resize(D2D1::SizeF(ScrollBarThickness, v)); }
void VerticalScrollBar::resizeContent(const D2D1_SIZE_F& size)
{
	if (!this->isValid()) return;

	auto scaling = static_cast<float>(this->getPageSize() / this->getFullSize());
	auto barLength = (size.height - (ThumbMargin + ScrollBarThickness) * 2.0f) * scaling;
	auto conLength = (size.height - (ThumbMargin + ScrollBarThickness) * 2.0f) - barLength;

	this->pPositiveButton->setOffset(D2D1::Point2F(0.0f, size.height - this->pPositiveButton->getSize().height));
	this->getThumb()->resize(D2D1::SizeF(size.width - ThumbMargin * 2.0f, barLength));
	this->getThumb()->setOffset(D2D1::Point2F(ThumbMargin, static_cast<float>(this->pNegativeButton->getBottom() + ThumbMargin + this->getCurrentPercent() * conLength)));
}
void VerticalScrollBar::updateAll()
{
	this->ScrollBarBase::updateAll();
	this->pNegativeButton->updateAll();
	this->pPositiveButton->updateAll();
}
void VerticalScrollBar::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->isValid() && this->pNegativeButton->hitTest(pt))
	{
		this->pNegativeButton->onMouseMove(this->pNegativeButton->toLocal(pt));
	}
	else if (this->isValid() && this->pPositiveButton->hitTest(pt))
	{
		this->pPositiveButton->onMouseMove(this->pPositiveButton->toLocal(pt));
	}
	else
	{
		this->ScrollBarBase::onMouseMove(pt);
	}
}
void VerticalScrollBar::startThumbDrag(const D2D1_POINT_2F& pt)
{
	this->clickOffset = pt.y;
}
void VerticalScrollBar::moveInThumb(const D2D1_POINT_2F& pt)
{
	auto pos = this->getThumb()->toParentLocal(pt).y - this->clickOffset;
	if (pos < this->pNegativeButton->getBottom() + ThumbMargin) pos = this->pNegativeButton->getBottom() + ThumbMargin;
	if (pos + this->getThumb()->getSize().height > this->pPositiveButton->getTop() - ThumbMargin)
	{
		pos = this->pPositiveButton->getTop() - ThumbMargin - this->getThumb()->getSize().height;
	}
	auto lpos = pos - (ScrollBarThickness + ThumbMargin);
	auto conLength = this->getSize().height - (ScrollBarThickness + ThumbMargin) * 2.0f - this->getThumb()->getSize().height;
	this->updateCurrentValue((lpos / conLength) * (this->getFullSize() - this->getPageSize()));
	this->getThumb()->setOffset(D2D1::Point2F(ThumbMargin, pos));
	getCurrentContext().getLayerManager()->getDevice()->Commit();
}
void VerticalScrollBar::endThumbDrag() {}
void VerticalScrollBar::validate()
{
	this->ScrollBarBase::validate();
	this->addChild(this->pNegativeButton.get());
	this->addChild(this->pPositiveButton.get());
}
void VerticalScrollBar::invalidate()
{
	this->ScrollBarBase::invalidate();
	this->removeChild(this->pNegativeButton.get());
	this->removeChild(this->pPositiveButton.get());
}

VerticalScrollBar::ScrollUpButton::ScrollUpButton() : ScrollUnitButton()
{

}
VerticalScrollBar::ScrollUpButton::~ScrollUpButton() = default;
void VerticalScrollBar::ScrollUpButton::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.75f));
	this->ScrollUnitButton::updateContent(pRenderContext);
	pRenderContext->drawStringCenter(L"\ue010", this->getLocalRect(), L"symDefault", pTextBrush.Get());
}
VerticalScrollBar::ScrollDownButton::ScrollDownButton() : ScrollUnitButton()
{

}
VerticalScrollBar::ScrollDownButton::~ScrollDownButton() = default;
void VerticalScrollBar::ScrollDownButton::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.75f));
	this->ScrollUnitButton::updateContent(pRenderContext);
	pRenderContext->drawStringCenter(L"\ue011", this->getLocalRect(), L"symDefault", pTextBrush.Get());
}

HorizontalScrollBar::HorizontalScrollBar(IScrollBarValueReceptor* pReceptor) : ScrollBarBase(pReceptor)
{
	this->pNegativeButton = std::make_unique<ScrollLeftButton>();
	this->pPositiveButton = std::make_unique<ScrollRightButton>();

	this->addChild(this->pNegativeButton.get());
	this->addChild(this->pPositiveButton.get());
}
HorizontalScrollBar::~HorizontalScrollBar() = default;
void HorizontalScrollBar::adjustWidth(float v) { this->resize(D2D1::SizeF(v, ScrollBarThickness)); }
void HorizontalScrollBar::resizeContent(const D2D1_SIZE_F& size)
{
	if (!this->isValid()) return;

	auto scaling = static_cast<float>(this->getPageSize() / this->getFullSize());
	auto barLength = (size.width - (ThumbMargin + ScrollBarThickness) * 2.0f) * scaling;
	auto conLength = (size.width - (ThumbMargin + ScrollBarThickness) * 2.0f) - barLength;

	this->pPositiveButton->setOffset(D2D1::Point2F(size.width - this->pPositiveButton->getSize().width, 0.0f));
	this->getThumb()->resize(D2D1::SizeF(barLength, size.height - ThumbMargin * 2.0f));
	this->getThumb()->setOffset(D2D1::Point2F(static_cast<float>(this->pNegativeButton->getRight() + ThumbMargin + this->getCurrentPercent() * conLength), ThumbMargin));
}
void HorizontalScrollBar::updateAll()
{
	this->ScrollBarBase::updateAll();
	this->pNegativeButton->updateAll();
	this->pPositiveButton->updateAll();
}
void HorizontalScrollBar::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->isValid() && this->pNegativeButton->hitTest(pt))
	{
		this->pNegativeButton->onMouseMove(this->pNegativeButton->toLocal(pt));
	}
	else if (this->isValid() && this->pPositiveButton->hitTest(pt))
	{
		this->pPositiveButton->onMouseMove(this->pPositiveButton->toLocal(pt));
	}
	else
	{
		this->ScrollBarBase::onMouseMove(pt);
	}
}
void HorizontalScrollBar::startThumbDrag(const D2D1_POINT_2F& pt)
{
	this->clickOffset = pt.x;
}
void HorizontalScrollBar::moveInThumb(const D2D1_POINT_2F& pt)
{
	auto pos = this->getThumb()->toParentLocal(pt).x - this->clickOffset;
	if (pos < this->pNegativeButton->getRight() + ThumbMargin) pos = this->pNegativeButton->getRight() + ThumbMargin;
	if (pos + this->getThumb()->getSize().width > this->pPositiveButton->getLeft() - ThumbMargin)
	{
		pos = this->pPositiveButton->getLeft() - ThumbMargin - this->getThumb()->getSize().width;
	}
	auto lpos = pos - (ScrollBarThickness + ThumbMargin);
	auto conLength = this->getSize().width - (ScrollBarThickness + ThumbMargin) * 2.0f - this->getThumb()->getSize().width;
	this->updateCurrentValue((lpos / conLength) * (this->getFullSize() - this->getPageSize()));
	this->getThumb()->setOffset(D2D1::Point2F(pos, ThumbMargin));
	getCurrentContext().getLayerManager()->getDevice()->Commit();
}
void HorizontalScrollBar::endThumbDrag() {}
void HorizontalScrollBar::validate()
{
	this->ScrollBarBase::validate();
	this->addChild(this->pNegativeButton.get());
	this->addChild(this->pPositiveButton.get());
}
void HorizontalScrollBar::invalidate()
{
	this->ScrollBarBase::invalidate();
	this->removeChild(this->pNegativeButton.get());
	this->removeChild(this->pPositiveButton.get());
}

HorizontalScrollBar::ScrollLeftButton::ScrollLeftButton() : ScrollUnitButton()
{

}
HorizontalScrollBar::ScrollLeftButton::~ScrollLeftButton() = default;
void HorizontalScrollBar::ScrollLeftButton::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.75f));
	this->ScrollUnitButton::updateContent(pRenderContext);
	pRenderContext->drawStringCenter(L"\ue00e", this->getLocalRect(), L"symDefault", pTextBrush.Get());
}
HorizontalScrollBar::ScrollRightButton::ScrollRightButton() : ScrollUnitButton()
{

}
HorizontalScrollBar::ScrollRightButton::~ScrollRightButton() = default;
void HorizontalScrollBar::ScrollRightButton::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.75f));
	this->ScrollUnitButton::updateContent(pRenderContext);
	pRenderContext->drawStringCenter(L"\ue00f", this->getLocalRect(), L"symDefault", pTextBrush.Get());
}

ScrollingThumb::ScrollingThumb(ScrollBarBase* pParent) : Layer(D2D1::SizeF(ScrollBarThickness - ThumbMargin * 2.0f, ScrollBarThickness - ThumbMargin * 2.0f))
{
	this->pParentObject = pParent;

	this->pClickableOverlay = std::make_unique<ClickableOverlay>();
	this->pPressingOverlay = std::make_unique<PressingOverlay>();

	this->addChild(this->pClickableOverlay.get());
	this->addChild(this->pPressingOverlay.get());
}
ScrollingThumb::~ScrollingThumb() = default;
void ScrollingThumb::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(ColorPalette::ScrollingThumbBack);
}
void ScrollingThumb::resizeContent(const D2D1_SIZE_F& size)
{
	this->pClickableOverlay->resize(size);
	this->pPressingOverlay->resize(size);
}
void ScrollingThumb::updateAll()
{
	this->Layer::updateAll();
	this->pClickableOverlay->updateAll();
}
void ScrollingThumb::onMouseMove(const D2D1_POINT_2F& pt)
{
	this->Layer::onMouseMove(pt);

	// peeking / tee routing
	this->lastOffset = pt;
	if (getCurrentContext().isHoldingLayer(this)) this->pParentObject->moveInThumb(pt);
}
void ScrollingThumb::onMouseEnter()
{
	this->pClickableOverlay->raiseEffect();
}
void ScrollingThumb::onMouseLeave()
{
	this->pClickableOverlay->falloffEffect();
}
void ScrollingThumb::onMouseDown()
{
	getCurrentContext().holdLayerWithEntering(this);
	this->pPressingOverlay->raiseEffect();
	this->pParentObject->startThumbDrag(this->lastOffset);
}
void ScrollingThumb::onMouseUp()
{
	getCurrentContext().holdLayerWithEntering(nullptr);
	this->pPressingOverlay->falloffEffect();
	if (!getCurrentContext().isEnteringLayer(this)) this->onMouseLeave();
	this->pParentObject->endThumbDrag();
}

ScrollingThumb::ClickableOverlay::ClickableOverlay() : Layer()
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
ScrollingThumb::ClickableOverlay::~ClickableOverlay() = default;
void ScrollingThumb::ClickableOverlay::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::Black, 0.25f));
}
void ScrollingThumb::ClickableOverlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.0625, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ScrollingThumb::ClickableOverlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}

ScrollingThumb::PressingOverlay::PressingOverlay() : Layer()
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
ScrollingThumb::PressingOverlay::~PressingOverlay() = default;
void ScrollingThumb::PressingOverlay::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::Black, 0.25f));
}
void ScrollingThumb::PressingOverlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.0625, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ScrollingThumb::PressingOverlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}

ScrollUnitButton::ScrollUnitButton() : Layer(D2D1::SizeF(ScrollBarThickness, ScrollBarThickness))
{
	this->pClickableOverlay = std::make_unique<ClickableOverlay>();
	this->pPressingOverlay = std::make_unique<PressingOverlay>();

	this->addChild(this->pClickableOverlay.get());
	this->addChild(this->pPressingOverlay.get());
}
ScrollUnitButton::~ScrollUnitButton() = default;
void ScrollUnitButton::resizeContent(const D2D1_SIZE_F& size)
{
	this->pClickableOverlay->resize(size);
	this->pPressingOverlay->resize(size);
}
void ScrollUnitButton::updateAll()
{
	this->Layer::updateAll();
	this->pClickableOverlay->updateAll();
	this->pPressingOverlay->updateAll();
}
void ScrollUnitButton::onMouseEnter()
{
	if (getCurrentContext().isHoldingLayer(this))
	{
		this->pPressingOverlay->raiseEffect();
	}
	else
	{
		this->pClickableOverlay->raiseEffect();
	}
}
void ScrollUnitButton::onMouseLeave()
{
	if (getCurrentContext().isHoldingLayer(this))
	{
		this->pPressingOverlay->falloffEffect();
	}
	else
	{
		this->pClickableOverlay->falloffEffect();
	}
}
void ScrollUnitButton::onMouseDown()
{
	this->pPressingOverlay->raiseEffect();
	getCurrentContext().holdLayer(this);
}
void ScrollUnitButton::onMouseUp()
{
	if (getCurrentContext().isEnteringLayer(this)) this->pPressingOverlay->falloffEffect();
	getCurrentContext().holdLayer(nullptr);
	if (!getCurrentContext().isEnteringLayer(this)) this->onMouseLeave();
}

ScrollUnitButton::ClickableOverlay::ClickableOverlay() : Layer(D2D1::SizeF(ScrollBarThickness, ScrollBarThickness))
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
ScrollUnitButton::ClickableOverlay::~ClickableOverlay() = default;
void ScrollUnitButton::ClickableOverlay::updateContent(RenderContext* pRenderContext)
{
	auto pBorderBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.5f));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawRectFrame(this->getLocalRectPixelated(), pBorderBrush.Get());
}
void ScrollUnitButton::ClickableOverlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.0625, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ScrollUnitButton::ClickableOverlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}

ScrollUnitButton::PressingOverlay::PressingOverlay() : Layer(D2D1::SizeF(ScrollBarThickness, ScrollBarThickness))
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
ScrollUnitButton::PressingOverlay::~PressingOverlay() = default;
void ScrollUnitButton::PressingOverlay::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::Black, 0.1875f));
}
void ScrollUnitButton::PressingOverlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.0625, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ScrollUnitButton::PressingOverlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}