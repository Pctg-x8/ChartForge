#include "chartEditor.h"

ChartEditor::ChartEditor() : Layer(D2D1::SizeF(100.0f, 100.0f))
{

}

ChartEditor::~ChartEditor() = default;

void ChartEditor::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::LimeGreen));
}