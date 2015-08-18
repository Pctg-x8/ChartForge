#include "quantizeSelectorPopup.h"
#include "appContext.h"
#include "comutils.h"
#include <initializer_list>

using namespace std::string_literals;

UnmanagedLayer::UnmanagedLayer(float initWidth, float initHeight)
{
	ComResult hr;

	hr = getCurrentContext().getLayerManager()->getDevice()->CreateVisual(&this->pVisual);
	hr = getCurrentContext().getLayerManager()->getDevice()->CreateSurface(static_cast<UINT>(initWidth), static_cast<UINT>(initHeight),
		DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, &this->pSurface);
	this->pVisual->SetContent(this->pSurface.Get());
}

QuantizeSelectorPopup::QuantizeSelectorPopup() : PopupFrame(L"QuantizeSelectorPopup", &QuantizeSelectorPopup::WndProc)
{
	ComResult hr;

	SetClassLong(this->getNativePointer(), GCL_STYLE, GetClassLong(this->getNativePointer(), GCL_STYLE) | CS_DROPSHADOW);
	SetWindowPos(this->getNativePointer(), nullptr, 0, 0, 110, static_cast<int>(PopupHeight),
		SWP_NOZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);

	this->pBackLayer = std::make_unique<UnmanagedLayer>(110.0f, PopupHeight);
	this->getCompTarget()->SetRoot(this->pBackLayer->pVisual.Get());
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacityEffect = std::make_unique<LayerOpacityEffect>();
	this->pBackLayer->pVisual->SetEffect(this->pOpacityEffect->getEffectObject());
	this->pOpacityEffect->setOpacity(0.0f);

	{
		ComPtr<ID2D1DeviceContext> pContext;
		POINT ptOffset;
		hr = this->pBackLayer->pSurface->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), &pContext, &ptOffset);
		{
			auto pRenderContext = std::make_unique<RenderContext>(pContext);
			auto pBorderBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(0xa0c0ff));

			pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::White, 0.75f));
			pRenderContext->drawRectFrame(D2D1::RectF(ptOffset.x + 0.5f, ptOffset.y + 0.5f, ptOffset.x + 110 - 0.5f, ptOffset.y + PopupHeight - 0.5f), pBorderBrush.Get());
		}
		hr = this->pBackLayer->pSurface->EndDraw();
	}

	auto ilist = QuantizeElements::getValueArray();
	for (auto i = 0; i < QuantizeElements::Size; i++)
	{
		this->pSelectorElements[i] = std::make_unique<SelectorElement>(ilist[i]);
		this->pBackLayer->pVisual->AddVisual(this->pSelectorElements[i]->getLabelLayer()->pVisual.Get(), true, nullptr);
		this->pSelectorElements[i]->getLabelLayer()->pVisual->SetOffsetY(i * QuantizeElementHeight);
		this->pBackLayer->pVisual->AddVisual(this->pSelectorElements[i]->getIndicateLayer()->pVisual.Get(), true, nullptr);
		this->pSelectorElements[i]->getIndicateLayer()->pVisual->SetOffsetY(i * QuantizeElementHeight);
	}

	this->lastSelectIndex = -1;
}
QuantizeSelectorPopup::~QuantizeSelectorPopup() = default;

LRESULT CALLBACK QuantizeSelectorPopup::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		// hide
		getCurrentContext().getQuantizeSelectorPopup()->hide();
		break;
	case WM_MOUSEMOVE:
		getCurrentContext().getQuantizeSelectorPopup()->updateSelectionIndicators(lParam);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void QuantizeSelectorPopup::updateSelectionIndicators(LPARAM lParam)
{
	auto posX = static_cast<int16_t>(LOWORD(lParam));
	auto posY = static_cast<int16_t>(HIWORD(lParam));

	if (posX < 0 || posX >= 110.0f || posY < 0 || posY >= PopupHeight)
	{
		if (this->lastSelectIndex >= 0) this->pSelectorElements[this->lastSelectIndex]->falloffIndicator();
		this->lastSelectIndex = -1;
	}
	else
	{
		auto newIndex = static_cast<int32_t>(floor(posY / QuantizeElementHeight));
		if (this->lastSelectIndex != newIndex)
		{
			if (this->lastSelectIndex >= 0) this->pSelectorElements[this->lastSelectIndex]->falloffIndicator();
			this->lastSelectIndex = newIndex;
			this->pSelectorElements[this->lastSelectIndex]->raiseIndicator();
		}
	}
}

void QuantizeSelectorPopup::onShow()
{
	SetCapture(this->getNativePointer());
	this->pAlphaAnimator->setLinear(0.125, 0.0f, 1.0f);
	this->pOpacityEffect->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();

	for (auto i = 0; i < QuantizeElements::Size; i++)
	{
		this->pSelectorElements[i]->raiseEffect(i * (1.0 / 64.0));
	}
}
void QuantizeSelectorPopup::onHide()
{
	ReleaseCapture();
	this->pAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
	this->pOpacityEffect->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();

	if (this->lastSelectIndex >= 0)
	{
		this->pSelectorElements[this->lastSelectIndex]->falloffIndicator();
		auto ilist = QuantizeElements::getValueArray();
		getCurrentContext().changeQuantizeValue(ilist[this->lastSelectIndex]);
	}

	for (auto i = 0; i < QuantizeElements::Size; i++)
	{
		this->pSelectorElements[i]->falloffEffect();
	}
}

QuantizeSelectorPopup::SelectorElement::SelectorElement(double value)
{
	std::wstring label_str;
	if (value <= 1.0)
	{
		label_str = std::to_wstring(static_cast<uint32_t>(1.0 / value));
	}
	else
	{
		label_str = L"1/"s + std::to_wstring(static_cast<uint32_t>(value));
	}
	label_str.append(L" bars");
	this->pLabelLayer = std::make_unique<UnmanagedLayer>(110.0f, QuantizeElementHeight);
	this->pIndicateLayer = std::make_unique<UnmanagedLayer>(110.0f, QuantizeElementHeight);

	this->pLabelOpacityEffect = std::make_unique<LayerOpacityEffect>();
	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pHorizontalAnimator = std::make_unique<LayerAnimation>();
	this->pLabelLayer->pVisual->SetEffect(this->pLabelOpacityEffect->getEffectObject());
	this->pLabelOpacityEffect->setOpacity(0.0f);

	this->pIndicateAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pIndicateLayerOpacity = std::make_unique<LayerOpacityEffect>();
	this->pIndicateLayer->pVisual->SetEffect(this->pIndicateLayerOpacity->getEffectObject());
	this->pIndicateLayerOpacity->setOpacity(0.0f);

	ComResult hr;
	{
		ComPtr<ID2D1DeviceContext> pContext;
		POINT ptOffset;
		hr = this->pLabelLayer->pSurface->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), &pContext, &ptOffset);
		{
			auto pRenderContext = std::make_unique<RenderContext>(pContext);
			auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));

			pRenderContext->clearAsComponentBackground();
			pRenderContext->drawString(label_str, D2D1::Point2F(ptOffset.x + 5.5f, ptOffset.y + 2.5f), L"uiDefault", pTextBrush.Get());
		}
		hr = this->pLabelLayer->pSurface->EndDraw();
	}
	{
		ComPtr<ID2D1DeviceContext> pContext;
		POINT ptOffset;
		hr = this->pIndicateLayer->pSurface->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), &pContext, &ptOffset);
		{
			auto pRenderContext = std::make_unique<RenderContext>(pContext);
			pRenderContext->clear(D2D1::ColorF(0x007fff, 0.25f));
		}
		hr = this->pIndicateLayer->pSurface->EndDraw();
	}
	this->hasIndicatedSelection = false;
}
void QuantizeSelectorPopup::SelectorElement::raiseEffect(double delays)
{
	this->pAlphaAnimator->setDelayedLinear(delays, 0.125f, 0.0f, 1.0f);
	this->pHorizontalAnimator->setDelayedInverseQuadratic(delays, 0.125, 10.0f, 0.0f);
	this->pLabelOpacityEffect->animateOpacity(this->pAlphaAnimator.get());
	this->pLabelLayer->pVisual->SetOffsetX(this->pHorizontalAnimator->getAnimationObject());
	this->pAlphaAnimator->commit();
}
void QuantizeSelectorPopup::SelectorElement::falloffEffect()
{
	this->pAlphaAnimator->setLinear(0.125f, 1.0f, 0.0f);
	this->pLabelOpacityEffect->animateOpacity(this->pAlphaAnimator.get());
	this->pAlphaAnimator->commit();
}
void QuantizeSelectorPopup::SelectorElement::raiseIndicator()
{
	if (!this->hasIndicatedSelection)
	{
		this->pIndicateAlphaAnimator->setLinear(0.0625, 0.0f, 1.0f);
		this->pIndicateLayerOpacity->animateOpacity(this->pIndicateAlphaAnimator.get());
		this->pIndicateAlphaAnimator->commit();
	}
	
	this->hasIndicatedSelection = true;
}
void QuantizeSelectorPopup::SelectorElement::falloffIndicator()
{
	if (this->hasIndicatedSelection)
	{
		this->pIndicateAlphaAnimator->setLinear(0.125, 1.0f, 0.0f);
		this->pIndicateLayerOpacity->animateOpacity(this->pIndicateAlphaAnimator.get());
		this->pIndicateAlphaAnimator->commit();
	}

	this->hasIndicatedSelection = false;
}