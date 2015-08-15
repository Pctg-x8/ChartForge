#pragma once

#include "layerManager.h"

class ChartEditor : public Layer
{
public:
	ChartEditor();
	~ChartEditor();
protected:
	void updateContent(RenderContext* pRenderContext) override;
};