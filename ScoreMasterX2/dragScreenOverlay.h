#pragma once

#include "layerManager.h"
#include <mfidl.h>
#include <string>

class DragScreenOverlay : public Layer
{
	bool effectRaised;

	std::unique_ptr<LayerAnimation> pAlphaAnimator;
	std::unique_ptr<LayerOpacityEffect> pOpacity;
public:
	DragScreenOverlay();
	~DragScreenOverlay();

	auto isEffectRaised() { return this->effectRaised; }

	void raiseEffect();
	void falloffEffect();
protected:
	void updateContent(RenderContext* pRenderContext) override;
};