#include "arrangerToolComponents.h"
#include "appContext.h"

#include "colorPalette.h"

ToolCommandButton::ToolCommandButton() : Layer(D2D1::SizeF(ToolsSize, ToolsSize))
{
	this->pClickableOverlay = std::make_unique<ClickableOverlay>();
	this->pPressingOverlay = std::make_unique<PressingOverlay>();

	this->addChild(this->pClickableOverlay.get());
	this->addChild(this->pPressingOverlay.get());
}
ToolCommandButton::~ToolCommandButton() = default;
void ToolCommandButton::updateContent(RenderContext* pRenderContext)
{
	this->Layer::updateContent(pRenderContext);
}
void ToolCommandButton::resizeContent(const D2D1_SIZE_F& size)
{
	this->pClickableOverlay->resize(size);
	this->pPressingOverlay->resize(size);
}

void ToolCommandButton::onMouseEnter()
{
	if (!getCurrentContext().isHoldingLayer(this))
	{
		this->pClickableOverlay->raiseEffect();
	}
	else
	{
		this->pPressingOverlay->raiseEffect();
	}
}
void ToolCommandButton::onMouseLeave()
{
	if (!getCurrentContext().isHoldingLayer(this))
	{
		this->pClickableOverlay->falloffEffect();
	}
	else
	{
		this->pPressingOverlay->falloffEffect();
	}
}
void ToolCommandButton::onMouseDown()
{
	this->pPressingOverlay->raiseEffect();
	getCurrentContext().holdLayer(this);
}
void ToolCommandButton::onMouseUp()
{
	getCurrentContext().holdLayer(nullptr);
	if (getCurrentContext().isEnteringLayer(this))
	{
		this->pPressingOverlay->falloffEffect();
		this->onClick();
	}
	else
	{
		this->onMouseLeave();
	}
}

ToolCommandButton::ClickableOverlay::ClickableOverlay() : Layer(D2D1::SizeF(ToolsSize, ToolsSize))
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->pOpacity->setOpacity(0.0f);
	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
}
ToolCommandButton::ClickableOverlay::~ClickableOverlay() = default;
void ToolCommandButton::ClickableOverlay::updateContent(RenderContext* pRenderContext)
{
	auto pBorderBrush = pRenderContext->createSolidColorBrush(ColorPalette::ButtonClickableBorder);

	pRenderContext->clear(ColorPalette::ButtonSelectedFill);
	pRenderContext->drawRectFrame(this->getLocalRectPixelated(), pBorderBrush.Get());
}

void ToolCommandButton::ClickableOverlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.125, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ToolCommandButton::ClickableOverlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.1875, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}

ToolCommandButton::PressingOverlay::PressingOverlay() : Layer(D2D1::SizeF(ToolsSize, ToolsSize))
{
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->pOpacity->setOpacity(0.0f);
	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
}
ToolCommandButton::PressingOverlay::~PressingOverlay() = default;
void ToolCommandButton::PressingOverlay::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(ColorPalette::ButtonSelectedFill);
}

void ToolCommandButton::PressingOverlay::raiseEffect()
{
	this->pAlphaAnimator->setLinear(0.0625, 0.0f, 1.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void ToolCommandButton::PressingOverlay::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
	this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}