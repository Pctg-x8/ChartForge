#pragma once

#include "layerManager.h"
#include <memory>
#include <string>

enum class SelectorIds
{
	ArrangerView = 1u, ChartEditor
};

class SelectorComponent : public Layer
{
	class ClickableOverlay : public Layer
	{
		std::unique_ptr<LayerAnimation> pAlphaAnimator;
		std::unique_ptr<LayerOpacityEffect> pOpacityProperty;
	public:
		ClickableOverlay(const D2D1_SIZE_F& initSize);
		virtual ~ClickableOverlay();

		auto getAlphaAnimator() { return this->pAlphaAnimator.get(); }
		void commitAnimation();

		void updateContent(RenderContext* pRenderContext) override;
	};
	class SelectionOverlay : public Layer
	{
		std::unique_ptr<LayerAnimation> pAlphaAnimator;
		std::unique_ptr<LayerOpacityEffect> pOpacityProperty;
	public:
		SelectionOverlay(const D2D1_SIZE_F& initSize);
		virtual ~SelectionOverlay();

		auto getAlphaAnimator() { return this->pAlphaAnimator.get(); }
		void commitAnimation();

		void updateContent(RenderContext* pRenderContext) override;
	};

	std::wstring label;
	SelectorIds id;
	std::unique_ptr<ClickableOverlay> pClickableOverlay;
	std::unique_ptr<SelectionOverlay> pOverlay;

	float calcWidth();
public:
	SelectorComponent(const std::wstring& label, SelectorIds id);
	virtual ~SelectorComponent();

	void updateContent(RenderContext* pRenderContext) override;
	void updateAll() override;
	void onMouseEnter() override;
	void onMouseLeave() override;
	void onMouseDown() override;

	void select();
	void deselect();
};

class ScreenSelector : public Layer
{
	std::unique_ptr<SelectorComponent> pArrangerScreenSelector, pChartScreenSelector;

	SelectorIds currentSelectionId;
public:
	ScreenSelector();
	virtual ~ScreenSelector();

	void adjustWidth(float width);
	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;

	void changeSelection(SelectorIds id);
};