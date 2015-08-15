#include "arrangerPage.h"
#include "appContext.h"
#include "arrangerView.h"
#include "wavePaletteView.h"

const auto SplitterSize = 6.0f;

ArrangerPage::ArrangerPage() : Layer()
{
	this->pArrangerView = std::make_unique<ArrangerView>();
	this->pSplitter = std::make_unique<Splitter>(this);
	this->pWavePaletteView = std::make_unique<WavePaletteView>();

	this->layoutRight = 240.0f;

	this->addChild(this->pArrangerView.get());
	this->addChild(this->pSplitter.get());
	this->addChild(this->pWavePaletteView.get());
}
ArrangerPage::~ArrangerPage() = default;
void ArrangerPage::updateAll()
{
	this->Layer::updateAll();
	this->pArrangerView->updateAll();
	this->pSplitter->updateAll();
	this->pWavePaletteView->updateAll();
}
void ArrangerPage::resizeContent(const D2D1_SIZE_F& size)
{
	if (size.width - this->layoutRight < 120.0f) this->layoutRight = size.width - 120.0f;
	if (this->layoutRight < 120.0f) this->layoutRight = 120.0f;
	this->pSplitter->setLeft(size.width - this->layoutRight - SplitterSize);
	this->pArrangerView->resize(D2D1::SizeF(this->pSplitter->getLeft(), size.height));
	this->pSplitter->adjustHeight(size.height);
	this->pWavePaletteView->setOffset(D2D1::Point2F(this->pSplitter->getRight(), 0.0f));
	this->pWavePaletteView->resize(D2D1::SizeF(size.width - this->pSplitter->getRight(), size.height));
}
void ArrangerPage::onMouseMove(const D2D1_POINT_2F& ps)
{
	if (this->pArrangerView->hitTest(ps))
	{
		this->pArrangerView->onMouseMove(this->pArrangerView->toLocal(ps));
	}
	else if (this->pSplitter->hitTest(ps))
	{
		this->pSplitter->onMouseMove(this->pSplitter->toLocal(ps));
	}
	else if (this->pWavePaletteView->hitTest(ps))
	{
		this->pWavePaletteView->onMouseMove(this->pWavePaletteView->toLocal(ps));
	}
	else
	{
		this->Layer::onMouseMove(ps);
	}
}
void ArrangerPage::startLayouting(const D2D1_POINT_2F& pt)
{
	this->layoutOffset = pt.x;
}
void ArrangerPage::moveSplitter(const D2D1_POINT_2F& pt)
{
	this->layoutRight = this->getSize().width - (this->pSplitter->toParentLocal(pt).x - this->layoutOffset + SplitterSize);
	this->Layer::resizeContent();
}

ArrangerPage::Splitter::Overlay::Overlay() : Layer()
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
ArrangerPage::Splitter::Overlay::~Overlay() = default;
void ArrangerPage::Splitter::Overlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.0625f, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ArrangerPage::Splitter::Overlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125f, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ArrangerPage::Splitter::Overlay::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::Black, 0.25f));
}

ArrangerPage::Splitter::Splitter(ArrangerPage* pParent) : Layer()
{
	this->pParentObject = pParent;

	this->pClickableOverlay = std::make_unique<Overlay>();
	this->pPressingOverlay = std::make_unique<Overlay>();

	this->addChild(this->pClickableOverlay.get());
	this->addChild(this->pPressingOverlay.get());
}
ArrangerPage::Splitter::~Splitter() = default;
void ArrangerPage::Splitter::adjustHeight(float h)
{
	this->resize(D2D1::SizeF(SplitterSize, h));
	this->pClickableOverlay->resize(D2D1::SizeF(SplitterSize, h));
	this->pPressingOverlay->resize(D2D1::SizeF(SplitterSize, h));
}
void ArrangerPage::Splitter::updateAll()
{
	this->Layer::updateAll();
	this->pClickableOverlay->updateAll();
	this->pPressingOverlay->updateAll();
}

void ArrangerPage::Splitter::onMouseMove(const D2D1_POINT_2F& pt)
{
	this->lastOffset = pt;
	if (getCurrentContext().isHoldingLayer(this))
	{
		this->pParentObject->moveSplitter(pt);
	}
	this->Layer::onMouseMove(pt);
}
void ArrangerPage::Splitter::onMouseEnter()
{
	this->pClickableOverlay->raiseEffect();
}
void ArrangerPage::Splitter::onMouseLeave()
{
	this->pClickableOverlay->falloffEffect();
}
void ArrangerPage::Splitter::onMouseDown()
{
	this->pPressingOverlay->raiseEffect();
	this->pParentObject->startLayouting(this->lastOffset);
	getCurrentContext().holdLayerWithEntering(this);
}
void ArrangerPage::Splitter::onMouseUp()
{
	this->pPressingOverlay->falloffEffect();
	getCurrentContext().holdLayerWithEntering(nullptr);
	if (!getCurrentContext().isEnteringLayer(this)) this->onMouseLeave();
}
