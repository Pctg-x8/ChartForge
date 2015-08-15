#include "backgroundLayer.h"
#include "renderDevice.h"

void BackgroundLayer::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clearAsAppBackground();
}