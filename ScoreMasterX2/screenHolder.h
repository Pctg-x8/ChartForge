#pragma once

#include "layerManager.h"
#include "appContext.h"

const auto ContentMargin = 16u;

template<typename ContentLayerT>
class ScreenHolder : public Layer
{
	std::unique_ptr<ContentLayerT> content;
	std::unique_ptr<LayerAnimation> pAlphaAnimator, pSlidingAnimator;
	std::unique_ptr<LayerOpacityEffect> pOpacity;
	bool is_shown;

	void applyAnimation()
	{
		this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
		this->getVisual()->SetOffsetX(this->pSlidingAnimator->getAnimationObject());
		getCurrentContext().getLayerManager()->getDevice()->Commit();
	}
public:
	ScreenHolder() : Layer(D2D1::SizeF(100.0f, 100.0f))
	{
		this->content = std::make_unique<ContentLayerT>();
		this->pAlphaAnimator = std::make_unique<LayerAnimation>();
		this->pSlidingAnimator = std::make_unique<LayerAnimation>();
		this->pOpacity = std::make_unique<LayerOpacityEffect>();
		this->addChild(this->content.get());

		this->content->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	}
	~ScreenHolder() = default;

	void setInitialState(bool shown)
	{
		if (shown) this->pOpacity->setOpacity(1.0f);
		else this->pOpacity->setOpacity(0.0f);
		this->is_shown = shown;
	}
	void show()
	{
		this->pAlphaAnimator->setLinear(0.25, 0.0f, 1.0f);
		this->pSlidingAnimator->setInverseQuadratic(0.25, 40.0f, 0.0f);
		this->applyAnimation();
		this->is_shown = true;
	}
	void hide()
	{
		this->pAlphaAnimator->setLinear(0.25, 1.0f, 0.0f);
		this->pSlidingAnimator->setLinear(0.25, 0.0f, -40.0f);
		this->applyAnimation();
		this->is_shown = false;
	}

	auto getContent() { return this->content.get(); }
	auto isShown() { return this->is_shown; }

	void onMouseMove(const D2D1_POINT_2F& pt) override
	{
		if (this->content->hitTest(pt))
		{
			this->content->onMouseMove(this->content->toLocal(pt));
		}
		else
		{
			this->Layer::onMouseMove(pt);
		}
	}
protected:
	virtual void resizeContent(const D2D1_SIZE_F& size) override
	{
		this->content->resize(D2D1::SizeF(size.width - ContentMargin * 2.0f, size.height - ContentMargin * 2.0f));
		this->content->setOffset(D2D1::Point2F(static_cast<float>(ContentMargin), static_cast<float>(ContentMargin)));
	}
};