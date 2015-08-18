#include "layerManager.h"
#include "appContext.h"

#pragma comment(lib, "dcomp")

#include "comutils.h"

bool operator==(const D2D1_SIZE_F& s1, const D2D1_SIZE_F& s2)
{
	return s1.width == s2.width && s1.height == s2.height;
}

static const auto DefaultLayerFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
static const auto DefaultLayerAlphaFormat = DXGI_ALPHA_MODE_PREMULTIPLIED;

LayerAnimation::LayerAnimation()
{
	ComResult hr;

	hr = getCurrentContext().getLayerManager()->getDevice()->CreateAnimation(&this->pAnimationObject);
}
LayerAnimation::~LayerAnimation() = default;
void LayerAnimation::setLinear(double seconds, float from, float to)
{
	this->pAnimationObject->Reset();
	this->pAnimationObject->AddCubic(0.0f, from, static_cast<float>((to - from) / seconds), 0.0f, 0.0f);
	this->pAnimationObject->End(seconds, to);
}
void LayerAnimation::setDelayedLinear(double delay, double seconds, float from, float to)
{
	this->pAnimationObject->Reset();
	this->pAnimationObject->AddCubic(0.0, from, 0.0f, 0.0f, 0.0f);
	this->pAnimationObject->AddCubic(delay, from, static_cast<float>((to - from) / seconds), 0.0f, 0.0f);
	this->pAnimationObject->End(delay + seconds, to);
}
void LayerAnimation::setQuadratic(double seconds, float from, float to)
{
	auto aval = static_cast<float>((to - from) / pow(seconds, 2.0f));
	this->pAnimationObject->Reset();
	this->pAnimationObject->AddCubic(0.0f, from, 0.0f, aval, 0.0f);
	this->pAnimationObject->End(seconds, to);
}
void LayerAnimation::setInverseQuadratic(double seconds, float from, float to)
{
	auto aval = static_cast<float>((from - to) / pow(seconds, 2.0f));
	auto bval = static_cast<float>(2.0f * (to - from) / seconds);
	auto cval = from;
	this->pAnimationObject->Reset();
	this->pAnimationObject->AddCubic(0.0f, cval, bval, aval, 0.0f);
	this->pAnimationObject->End(seconds, to);
}
void LayerAnimation::setDelayedInverseQuadratic(double delay, double seconds, float from, float to)
{
	auto aval = static_cast<float>((from - to) / pow(seconds, 2.0f));
	auto bval = static_cast<float>(2.0f * (to - from) / seconds);
	auto cval = from;
	this->pAnimationObject->Reset();
	this->pAnimationObject->AddCubic(0.0, from, 0.0f, 0.0f, 0.0f);
	this->pAnimationObject->AddCubic(delay, cval, bval, aval, 0.0f);
	this->pAnimationObject->End(delay + seconds, to);
}
void LayerAnimation::commit()
{
	getCurrentContext().getLayerManager()->getDevice()->Commit();
}

LayerOpacityEffect::LayerOpacityEffect()
{
	ComResult hr;

	hr = getCurrentContext().getLayerManager()->getDevice()->CreateEffectGroup(&this->pEffectObject);
}
LayerOpacityEffect::~LayerOpacityEffect() = default;

Layer::Layer(const D2D1_SIZE_F& size)
{
	ComResult hr;

	this->internalVisualOffset = D2D1::Point2F(0.0f, 0.0f);

	hr = getCurrentContext().getLayerManager()->getDevice()->CreateVisual(&this->pVisual);
	this->resize(size);
}
Layer::~Layer() = default;

void Layer::setOffset(const D2D1_POINT_2F& offs)
{
	this->pVisual->SetOffsetX(offs.x);
	this->pVisual->SetOffsetY(offs.y);
	this->internalVisualOffset = offs;
}
void Layer::setTop(float top)
{
	this->internalVisualOffset.y = top;
	this->pVisual->SetOffsetY(top);
}
void Layer::setLeft(float left)
{
	this->internalVisualOffset.x = left;
	this->pVisual->SetOffsetX(left);
}
void Layer::resize(const D2D1_SIZE_F& size)
{
	ComResult hr;

	if (this->internalSurfaceSize == size) return;

	if(this->pSurface)
	{
		if (size.width <= 1.0f || size.height <= 1.0f)
		{
			// remove surface
			hr = this->pVisual->SetContent(nullptr);
			this->pSurface.Reset();
		}
		else
		{
			this->pSurface->Resize(static_cast<UINT>(size.width), static_cast<UINT>(size.height));
			getCurrentContext().queueUpdated(this);
		}
	}
	else
	{
		if (size.width > 0.0f && size.height > 0.0f)
		{
			// make surface
			hr = getCurrentContext().getLayerManager()->getDevice()->CreateVirtualSurface(
				static_cast<UINT>(size.width), static_cast<UINT>(size.height),
				DefaultLayerFormat, DefaultLayerAlphaFormat, &this->pSurface);
			hr = this->pVisual->SetContent(this->pSurface.Get());
			getCurrentContext().queueUpdated(this);
		}
	}

	this->internalSurfaceSize = size;
	this->resizeContent(size);
}
void Layer::updateEntry()
{
	ComPtr<ID2D1DeviceContext> pRenderContext;
	POINT ptOffset;
	ComResult hr;
	std::unique_ptr<RenderContext> pRenderContextWrapped;

	if (!this->pSurface) return;
	if (this->internalSurfaceSize.width < 1.0f || this->internalSurfaceSize.height < 1.0f) return;

	hr = this->pSurface->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), &pRenderContext, &ptOffset);
	pRenderContext->SetTransform(D2D1::Matrix3x2F::Translation(static_cast<float>(ptOffset.x), static_cast<float>(ptOffset.y)));
	pRenderContextWrapped = std::make_unique<RenderContext>(pRenderContext);
	this->updateContent(pRenderContextWrapped.get());
	hr = this->pSurface->EndDraw();
}
void Layer::updateContent(RenderContext* pRenderContext)
{
	pRenderContext->clearAsComponentBackground();
}
void Layer::addChild(Layer* pLayer)
{
	if (pLayer->hasParent()) throw _com_error(E_INVALIDARG);
	this->pVisual->AddVisual(pLayer->getVisual(), false, nullptr);
	pLayer->pParentLayer = this;
}
void Layer::removeChild(Layer* pLayer)
{
	if (!pLayer->hasParent() || pLayer->getParent() != this) throw _com_error(E_INVALIDARG);
	this->pVisual->RemoveVisual(pLayer->getVisual());
	pLayer->pParentLayer = nullptr;
}
void Layer::updateAll()
{
	getCurrentContext().queueUpdated(this);
}
void Layer::onMouseMove(const D2D1_POINT_2F& pt)
{
	// pt: local point of mouse cursor
	getCurrentContext().notifyEnteredLayer(this);
	return;
}

LayerManager::LayerManager() = default;
LayerManager::~LayerManager() = default;

void LayerManager::initDevices()
{
	ComResult hr;

	hr = DCompositionCreateDevice2(getCurrentContext().getRenderDevice()->getDevice2(),
		__uuidof(IDCompositionDesktopDevice), &this->pDevice);
	hr = this->pDevice->CreateTargetForHwnd(getCurrentContext().getNativePointer(), true, &this->pTarget);
}

ComPtr<IDCompositionTarget> LayerManager::createTarget(HWND hNativePointer)
{
	ComResult hr;
	ComPtr<IDCompositionTarget> pTargetTemp;

	hr = this->pDevice->CreateTargetForHwnd(hNativePointer, true, &pTargetTemp);
	return pTargetTemp;
}