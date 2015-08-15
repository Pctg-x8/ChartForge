#pragma once

#include <windows.h>
#include <dcomp.h>
#include <wrl.h>
#include <memory>
#include "renderDevice.h"

#include "comutils.h"

using Microsoft::WRL::ComPtr;

class LayerAnimation
{
	ComPtr<IDCompositionAnimation> pAnimationObject;
public:
	LayerAnimation();
	~LayerAnimation();

	auto getAnimationObject() { return this->pAnimationObject.Get(); }

	void commit();
	void setLinear(double seconds, float from, float to);
	void setQuadratic(double seconds, float from, float to);
	void setInverseQuadratic(double seconds, float from, float to);
};
class LayerOpacityEffect
{
	ComPtr<IDCompositionEffectGroup> pEffectObject;
public:
	LayerOpacityEffect();
	~LayerOpacityEffect();

	void setOpacity(float v) { this->pEffectObject->SetOpacity(v); }
	void animateOpacity(LayerAnimation* pAnimator) { this->pEffectObject->SetOpacity(pAnimator->getAnimationObject()); }

	auto getEffectObject() { return this->pEffectObject.Get(); }
};

class Layer
{
	ComPtr<IDCompositionVisual2> pVisual;
	ComPtr<IDCompositionVirtualSurface> pSurface;
	Layer* pParentLayer;

	D2D1_POINT_2F internalVisualOffset;
	D2D1_SIZE_F internalSurfaceSize;
public:
	Layer(const D2D1_SIZE_F& size = D2D1_SIZE_F());
	virtual ~Layer();

	auto getVisual() { return this->pVisual.Get(); }

	void setOffset(const D2D1_POINT_2F& offs);
	void setTop(float top);
	void setLeft(float left);
	void resize(const D2D1_SIZE_F& size);
	void updateEntry();
	void addChild(Layer* pl);
	void removeChild(Layer* pl);

	auto getLeft() { return this->internalVisualOffset.x; }
	auto getTop() { return this->internalVisualOffset.y; }
	auto getRight() { return this->getLeft() + this->internalSurfaceSize.width; }
	auto getBottom() { return this->getTop() + this->internalSurfaceSize.height; }
	auto getSize() { return this->internalSurfaceSize; }
	auto getLocalRect() { return D2D1::RectF(0.0f, 0.0f, this->internalSurfaceSize.width, this->internalSurfaceSize.height); }
	auto getLocalRectPixelated()
	{
		return D2D1::RectF(0.5f, 0.5f, this->internalSurfaceSize.width - 0.5f, this->internalSurfaceSize.height - 0.5f);
	}
	auto hitTest(const D2D1_POINT_2F& ptOffsetParent)
	{
		return this->getLeft() <= ptOffsetParent.x && ptOffsetParent.x <= this->getRight()
			&& this->getTop() <= ptOffsetParent.y && ptOffsetParent.y <= this->getBottom();
	}
	auto toLocal(const D2D1_POINT_2F& ptOffsetParent)
	{
		return D2D1::Point2F(ptOffsetParent.x - this->getLeft(), ptOffsetParent.y - this->getTop());
	}
	auto hasParent() { return this->pParentLayer != nullptr; }
	auto getParent() { return this->pParentLayer; }
	D2D1_POINT_2F fromGlobal(const D2D1_POINT_2F& ptOffsetGlobal)
	{
		if (this->hasParent()) return this->toLocal(this->getParent()->fromGlobal(ptOffsetGlobal));
		else return this->toLocal(ptOffsetGlobal);
	}
	D2D1_POINT_2F toParentLocal(const D2D1_POINT_2F& ptOffsetLocal)
	{
		return D2D1::Point2F(ptOffsetLocal.x + this->getLeft(), ptOffsetLocal.y + this->getTop());
	}

	virtual void updateAll();
	virtual void onMouseMove(const D2D1_POINT_2F& pt);
	virtual void onMouseEnter() {}
	virtual void onMouseLeave() {}
	virtual void onMouseDown() {}
	virtual void onMouseUp() {}
	virtual void onClick() {}
protected:
	virtual void updateContent(RenderContext* pRenderContext);
	void resizeContent() { this->resizeContent(this->getSize()); }
	virtual void resizeContent(const D2D1_SIZE_F& size) {}
};

class LayerManager
{
	ComPtr<IDCompositionDesktopDevice> pDevice;
	ComPtr<IDCompositionTarget> pTarget;
	std::unique_ptr<Layer> pRootLayer;

	void initDevices();
public:
	LayerManager();
	~LayerManager();

	template<typename BackLayerT = Layer>
	void init()
	{
		ComResult hr;
		this->initDevices();
		this->pRootLayer = std::make_unique<BackLayerT>();
		hr = this->pTarget->SetRoot(this->pRootLayer->getVisual());
	}

	auto getDevice() { return this->pDevice.Get(); }
	auto getRootLayer() { return this->pRootLayer.get(); }
};