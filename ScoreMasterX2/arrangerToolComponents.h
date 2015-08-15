#pragma once

#include "layerManager.h"

const auto ToolsSize = 20.0f;
const auto ToolSpacing = 8.0f;

class ToolCommandButton : public Layer
{
	class ClickableOverlay : public Layer
	{
		std::unique_ptr<LayerAnimation> pAlphaAnimator;
		std::unique_ptr<LayerOpacityEffect> pOpacity;
	public:
		ClickableOverlay();
		~ClickableOverlay();

		void raiseEffect();
		void falloffEffect();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};
	class PressingOverlay : public Layer
	{
		std::unique_ptr<LayerAnimation> pAlphaAnimator;
		std::unique_ptr<LayerOpacityEffect> pOpacity;
	public:
		PressingOverlay();
		~PressingOverlay();

		void raiseEffect();
		void falloffEffect();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	std::unique_ptr<ClickableOverlay> pClickableOverlay;
	std::unique_ptr<PressingOverlay> pPressingOverlay;
public:
	ToolCommandButton();
	virtual ~ToolCommandButton();

	void onMouseEnter() override final;
	void onMouseLeave() override final;
	void onMouseDown() override final;
	void onMouseUp() override final;

	virtual void onClick() {}
protected:
	void updateContent(RenderContext* pRenderContext) override;
	void resizeContent(const D2D1_SIZE_F& size) override;
};