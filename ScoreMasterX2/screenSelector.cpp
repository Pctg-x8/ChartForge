#include "screenSelector.h"
#include "appContext.h"

#include "colorPalette.h"
#include <cassert>

static const auto GenericHeight = 24.0f;
static const auto SelectorSpacing = 8.0f;

ScreenSelector::ScreenSelector() : Layer(D2D1::SizeF(100.0f, GenericHeight))
{
	this->pArrangerScreenSelector = std::make_unique<SelectorComponent>(L"Arranger View", SelectorIds::ArrangerView);
	this->pChartScreenSelector = std::make_unique<SelectorComponent>(L"Chart Editor", SelectorIds::ChartEditor);

	this->addChild(this->pArrangerScreenSelector.get());
	this->addChild(this->pChartScreenSelector.get());

	this->pArrangerScreenSelector->setOffset(D2D1::Point2F(SelectorSpacing * 2.0f, 0.0f));
	this->pChartScreenSelector->setOffset(D2D1::Point2F(this->pArrangerScreenSelector->getRight() + SelectorSpacing, 0.0f));

	this->currentSelectionId = SelectorIds::ArrangerView;
	this->pArrangerScreenSelector->select();
}
ScreenSelector::~ScreenSelector() = default;

void ScreenSelector::adjustWidth(float width)
{
	this->resize(D2D1::SizeF(width, GenericHeight));
}
void ScreenSelector::updateAll()
{
	this->Layer::updateAll();
	this->pArrangerScreenSelector->updateAll();
	this->pChartScreenSelector->updateAll();
}
void ScreenSelector::onMouseMove(const D2D1_POINT_2F& pt)
{
	if (this->pArrangerScreenSelector->hitTest(pt))
	{
		this->pArrangerScreenSelector->onMouseMove(this->pArrangerScreenSelector->toLocal(pt));
	}
	else if (this->pChartScreenSelector->hitTest(pt))
	{
		this->pChartScreenSelector->onMouseMove(this->pChartScreenSelector->toLocal(pt));
	}
	else
	{
		this->Layer::onMouseMove(pt);
	}
}
void ScreenSelector::changeSelection(SelectorIds id)
{
	if (this->currentSelectionId == id) return;

	this->currentSelectionId = id;
	getCurrentContext().changeScreen(this->currentSelectionId);
	switch (id)
	{
	case SelectorIds::ArrangerView:
		this->pArrangerScreenSelector->select();
		this->pChartScreenSelector->deselect();
		break;
	case SelectorIds::ChartEditor:
		this->pArrangerScreenSelector->deselect();
		this->pChartScreenSelector->select();
		break;
	default: assert(false);
	}
}

SelectorComponent::SelectorComponent(const std::wstring& label, SelectorIds id) : Layer(D2D1::SizeF(GenericHeight, GenericHeight))
{
	this->label = label;
	this->id = id;
	this->resize(D2D1::SizeF(this->calcWidth() + GenericHeight, GenericHeight));
	
	this->pClickableOverlay = std::make_unique<SelectorComponent::ClickableOverlay>(this->getSize());
	this->pOverlay = std::make_unique<SelectorComponent::SelectionOverlay>(this->getSize());
	this->addChild(this->pClickableOverlay.get());
	this->addChild(this->pOverlay.get());
}
SelectorComponent::~SelectorComponent() = default;
float SelectorComponent::calcWidth()
{
	auto v = getCurrentContext().getRenderDevice()->calcStringWidth(this->label, L"uiDefault");
	return ceil(v);
}
void SelectorComponent::updateAll()
{
	this->Layer::updateAll();

	this->pClickableOverlay->updateAll();
	this->pOverlay->updateAll();
}
void SelectorComponent::onMouseDown()
{
	getCurrentContext().getScreenSelector()->changeSelection(this->id);
}

void SelectorComponent::updateContent(RenderContext* pRenderContext)
{
	auto pLabelBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawString(this->label, D2D1::Point2F(GenericHeight / 2.0f, 3.5f), L"uiDefault", pLabelBrush.Get());
}
void SelectorComponent::select()
{
	this->pOverlay->getAlphaAnimator()->setLinear(0.125, 0.0f, 1.0f);
	this->pOverlay->commitAnimation();
}
void SelectorComponent::deselect()
{
	this->pOverlay->getAlphaAnimator()->setLinear(0.125, 1.0f, 0.0f);
	this->pOverlay->commitAnimation();
}
void SelectorComponent::onMouseEnter()
{
	this->pClickableOverlay->getAlphaAnimator()->setQuadratic(0.125, 0.0f, 1.0f);
	this->pClickableOverlay->commitAnimation();
}
void SelectorComponent::onMouseLeave()
{
	this->pClickableOverlay->getAlphaAnimator()->setQuadratic(0.1875, 1.0f, 0.0f);
	this->pClickableOverlay->commitAnimation();
}

SelectorComponent::ClickableOverlay::ClickableOverlay(const D2D1_SIZE_F& initSize) : Layer(initSize)
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacityProperty = std::make_unique<LayerOpacityEffect>();
	this->getVisual()->SetEffect(this->pOpacityProperty->getEffectObject());

	this->pOpacityProperty->getEffectObject()->SetOpacity(0.0f);
}
SelectorComponent::ClickableOverlay::~ClickableOverlay() = default;
void SelectorComponent::ClickableOverlay::updateContent(RenderContext* pRenderContext)
{
	auto pBorderBrush = pRenderContext->createSolidColorBrush(ColorPalette::ButtonClickableBorder);

	pRenderContext->clearAsComponentBackground();
	pRenderContext->drawRectFrame(this->getLocalRectPixelated(), pBorderBrush.Get());
}
void SelectorComponent::ClickableOverlay::commitAnimation()
{
	this->pOpacityProperty->getEffectObject()->SetOpacity(this->pAlphaAnimator->getAnimationObject());
	this->pAlphaAnimator->commit();
}

SelectorComponent::SelectionOverlay::SelectionOverlay(const D2D1_SIZE_F& initSize) : Layer(initSize)
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacityProperty = std::make_unique<LayerOpacityEffect>();
	this->pOpacityProperty->getEffectObject()->SetOpacity(this->pAlphaAnimator->getAnimationObject());
	this->getVisual()->SetEffect(this->pOpacityProperty->getEffectObject());

	this->pOpacityProperty->getEffectObject()->SetOpacity(0.0f);
}
SelectorComponent::SelectionOverlay::~SelectionOverlay() = default;
void SelectorComponent::SelectionOverlay::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(ColorPalette::ButtonSelectedFill);
}
void SelectorComponent::SelectionOverlay::commitAnimation()
{
	this->pOpacityProperty->getEffectObject()->SetOpacity(this->pAlphaAnimator->getAnimationObject());
	this->pAlphaAnimator->commit();
}