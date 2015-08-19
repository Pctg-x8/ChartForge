#include "exitWaitChip.h"

#include "appContext.h"
#include "comutils.h"

static const auto Height = 120;
static const auto Width = Height * 16 / 9;

ExitWaitChip::ExitWaitChip() : PopupFrame(L"ExitWaitChip", &ExitWaitChip::WndProc)
{
	ComResult hr;

	SetParent(this->getNativePointer(), nullptr);
	SetWindowLong(this->getNativePointer(), GWL_STYLE, GetWindowLong(this->getNativePointer(), GWL_STYLE) | WS_BORDER);
	SetWindowPos(this->getNativePointer(), nullptr, (GetSystemMetrics(SM_CXSCREEN) - Width) / 2, (GetSystemMetrics(SM_CYSCREEN) - Height) / 2, Width, Height,
		SWP_NOZORDER | SWP_FRAMECHANGED);

	hr = getCurrentContext().getLayerManager()->getDevice()->CreateVisual(&this->pVisual);
	hr = getCurrentContext().getLayerManager()->getDevice()->CreateSurface(Width, Height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, &this->pSurface);
	hr = this->pVisual->SetContent(this->pSurface.Get());
	hr = this->getCompTarget()->SetRoot(this->pVisual.Get());

	ComPtr<ID2D1DeviceContext> pContext;
	POINT ptOffset;
	hr = this->pSurface->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), &pContext, &ptOffset);
	{
		const auto offs_p = D2D1::Point2F(static_cast<float>(ptOffset.x), static_cast<float>(ptOffset.y));
		auto pRenderContext = std::make_unique<RenderContext>(pContext);
		auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));

		pRenderContext->clearAsAppBackground();
		pRenderContext->drawStringHCenter(L"ChartForge", offs_p.x, offs_p.x + static_cast<float>(Width), offs_p.y + 4.0f, L"uiDefault", pTextBrush.Get());
		pRenderContext->drawStringCenter(L"Waiting for internal tasks...",
			D2D1::RectF(offs_p.x, offs_p.y + 16.0f, offs_p.x + static_cast<float>(Width), offs_p.y + static_cast<float>(Height)), L"uiDefault", pTextBrush.Get());
	}
	hr = this->pSurface->EndDraw();
}

LRESULT ExitWaitChip::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}