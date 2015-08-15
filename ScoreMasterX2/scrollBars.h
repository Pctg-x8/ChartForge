#pragma once

#include "layerManager.h"

const auto ScrollBarThickness = 16.0f;

class ScrollBarBase;
class IScrollBarValueReceptor
{
public:
	virtual void receiveValueChanged(ScrollBarBase* pSender) = 0;
};

class ScrollingThumb final : public Layer
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

	ScrollBarBase* pParentObject;
	D2D1_POINT_2F lastOffset;

	std::unique_ptr<ClickableOverlay> pClickableOverlay;
	std::unique_ptr<PressingOverlay> pPressingOverlay;
public:
	ScrollingThumb(ScrollBarBase* pParentObject);
	virtual ~ScrollingThumb();

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override final;
	void onMouseEnter() override final;
	void onMouseLeave() override final;
	void onMouseDown() override final;
	void onMouseUp() override final;
protected:
	void updateContent(RenderContext* pRenderContext) override;
	void resizeContent(const D2D1_SIZE_F& size) override;
};
class ScrollUnitButton : public Layer
{
	class ClickableOverlay final : public Layer
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
	class PressingOverlay final : public Layer
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
	ScrollUnitButton();
	virtual ~ScrollUnitButton();

	void updateAll() override;
	void onMouseEnter() override final;
	void onMouseLeave() override final;
	void onMouseDown() override final;
	void onMouseUp() override final;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};

class ScrollBarBase : public Layer
{
	std::unique_ptr<ScrollingThumb> pThumb;
	IScrollBarValueReceptor* pReceptor;

	double pageSize, fullSize, currentValue;
	bool is_valid;
public:
	ScrollBarBase(IScrollBarValueReceptor* pReceptor);
	virtual ~ScrollBarBase();

	auto getThumb() { return this->pThumb.get(); }

	auto getPageSize() { return this->pageSize; }
	auto getFullSize() { return this->fullSize; }
	auto getCurrentValue() { return this->currentValue; }
	auto getCurrentPercent() { return this->currentValue / (this->fullSize - this->pageSize); }
	void updatePageProperty(double p, double f);
	void updatePageSize(double p);
	void updateFullSize(double f);

	auto isValid() { return this->is_valid; }

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
	virtual void startThumbDrag(const D2D1_POINT_2F& offset) = 0;
	virtual void moveInThumb(const D2D1_POINT_2F& offset) = 0;
	virtual void endThumbDrag() = 0;
	virtual void validate();
	virtual void invalidate();
protected:
	void updateCurrentValue(double v);
	void updateContent(RenderContext* pRenderContext) override;
private:
	void updateValidStatus();
};

class VerticalScrollBar : public ScrollBarBase
{
	class ScrollUpButton : public ScrollUnitButton
	{
	public:
		ScrollUpButton();
		~ScrollUpButton();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};
	class ScrollDownButton : public ScrollUnitButton
	{
	public:
		ScrollDownButton();
		~ScrollDownButton();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	std::unique_ptr<ScrollUpButton> pNegativeButton;
	std::unique_ptr<ScrollDownButton> pPositiveButton;
	float clickOffset;
public:
	VerticalScrollBar(IScrollBarValueReceptor* pReceptor);
	~VerticalScrollBar();

	void adjustHeight(float h);
	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
	void startThumbDrag(const D2D1_POINT_2F& offset) override;
	void moveInThumb(const D2D1_POINT_2F& offset) override;
	void endThumbDrag();

	void validate() override;
	void invalidate() override;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};

class HorizontalScrollBar : public ScrollBarBase
{
	class ScrollLeftButton : public ScrollUnitButton
	{
	public:
		ScrollLeftButton();
		~ScrollLeftButton();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};
	class ScrollRightButton : public ScrollUnitButton
	{
	public:
		ScrollRightButton();
		~ScrollRightButton();
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	std::unique_ptr<ScrollLeftButton> pNegativeButton;
	std::unique_ptr<ScrollRightButton> pPositiveButton;
	float clickOffset;
public:
	HorizontalScrollBar(IScrollBarValueReceptor* pReceptor);
	~HorizontalScrollBar();

	void adjustWidth(float w);
	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
	void startThumbDrag(const D2D1_POINT_2F& offset) override;
	void moveInThumb(const D2D1_POINT_2F& offset) override;
	void endThumbDrag();

	void validate() override;
	void invalidate() override;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};