#pragma once

#include "layerManager.h"

class BackgroundLayer : public Layer
{
protected:
	void updateContent(RenderContext* pRenderContext) override;
};