#include "dragScreenOverlay.h"

DragScreenOverlay::DragScreenOverlay() : Layer()
{
	this->effectRaised = false;

	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
DragScreenOverlay::~DragScreenOverlay() = default;

void DragScreenOverlay::updateContent(RenderContext* pRenderContext)
{
	auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.125f));

	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
	pRenderContext->drawStringCenter(L"Drop to import wav file(s)", this->getLocalRect(), L"uiDefaultBig", pTextBrush.Get());
}

void DragScreenOverlay::raiseEffect()
{
	if (!this->effectRaised)
	{
		this->pAlphaAnimator->setInverseQuadratic(0.25, 0.0f, 1.0f);
		this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
		this->pAlphaAnimator->commit();
	}
	this->effectRaised = true;
}
void DragScreenOverlay::falloffEffect()
{
	if (this->effectRaised)
	{
		this->pAlphaAnimator->setLinear(0.25, 1.0f, 0.0f);
		this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
		this->pAlphaAnimator->commit();
	}
	this->effectRaised = false;
}