#include "renderDevice.h"
#include "appContext.h"

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "comutils.h"
#include "colorPalette.h"

#undef max
#include <climits>

static const auto floatMax = std::numeric_limits<float>::max();

RenderDevice::RenderDevice() = default;
RenderDevice::~RenderDevice() = default;

void RenderDevice::init()
{
	ComResult hr;

	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
		nullptr, 0, D3D11_SDK_VERSION, &this->pDevice3, nullptr, &this->pDeviceContext3);
	hr = this->pDevice3.As(&this->pxDevice);

	hr = D2D1CreateDevice(this->pxDevice.Get(), D2D1::CreationProperties(
		D2D1_THREADING_MODE_MULTI_THREADED, D2D1_DEBUG_LEVEL_ERROR, D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS),
		&this->pDevice2);

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &this->pdwFactory);
	hr = this->pdwFactory->GetSystemFontCollection(&this->pSystemFontCollection);
}

void RenderDevice::stockFormat(const std::wstring& key, const std::wstring& familyName, float size,
	DWRITE_FONT_WEIGHT fontWeight)
{
	auto iter = this->dwfstock_keypool.insert(key);
	if (iter.second)
	{
		// inserted
		ComPtr<IDWriteTextFormat> pft;
		ComResult hr;

		hr = this->pdwFactory->CreateTextFormat(familyName.c_str(), this->pSystemFontCollection.Get(),
			fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size * (96.0f / 72.0f),
			L"ja-jp", &pft);
		this->dwFormatStock[&(*iter.first)].first.Attach(pft.Detach());
		OutputDebugString(L"registered to ");
		OutputDebugString(key.c_str());
		OutputDebugString(L"\n");
	}
}
IDWriteTextFormat* RenderDevice::getStockedFormat(const std::wstring& key)
{
	auto iter = this->dwfstock_keypool.find(key);
	if (iter != std::end(this->dwfstock_keypool))
	{
		return this->dwFormatStock[&(*iter)].first.Get();
	}
	else
	{
		OutputDebugString(L"failed to getting ");
		OutputDebugString(key.c_str());
		OutputDebugString(L"\n");
		return nullptr;
	}
}
IDWriteFont* RenderDevice::getStockedFont(const std::wstring& key)
{
	auto iter = this->dwfstock_keypool.find(key);
	if (iter != std::end(this->dwfstock_keypool))
	{
		return this->dwFormatStock[&(*iter)].second.Get();
	}
	else return nullptr;
}
float RenderDevice::calcStringWidth(const std::wstring& strMeasured, const std::wstring& key)
{
	auto pFormat = this->getStockedFormat(key);
	if (pFormat == nullptr) return 0;
	
	ComPtr<IDWriteTextLayout> pLayout;
	ComResult hr;
	DWRITE_TEXT_METRICS metrics;

	hr = this->pdwFactory->CreateTextLayout(strMeasured.c_str(), static_cast<UINT>(strMeasured.length()), pFormat,
		floatMax, floatMax, &pLayout);
	hr = pLayout->GetMetrics(&metrics);
	return metrics.width;
}
D2D1_SIZE_F RenderDevice::calcStringSize(const std::wstring& strMeasured, IDWriteTextFormat* pFormat)
{
	if (pFormat == nullptr) return D2D1::SizeF();

	ComPtr<IDWriteTextLayout> pLayout;
	ComResult hr;
	DWRITE_TEXT_METRICS metrics;

	hr = this->pdwFactory->CreateTextLayout(strMeasured.c_str(), static_cast<UINT>(strMeasured.length()), pFormat,
		floatMax, floatMax, &pLayout);
	hr = pLayout->GetMetrics(&metrics);
	return D2D1::SizeF(metrics.width, metrics.height);
}
float RenderDevice::calcBaselineAscent(const std::wstring& key)
{
	auto pFont = this->getStockedFont(key);
	if (pFont == nullptr) return 0;

	DWRITE_FONT_METRICS metrics;
	pFont->GetMetrics(&metrics);
	return metrics.ascent;
}

RenderContext::RenderContext(const ComPtr<ID2D1DeviceContext>& pContextBase) : pInternalContext(pContextBase)
{
}
RenderContext::~RenderContext() = default;

void RenderContext::clearAsAppBackground()
{
	this->clear(ColorPalette::AppBackground);
}
void RenderContext::clearAsComponentBackground()
{
	this->clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
}
void RenderContext::clear(const D2D1_COLOR_F& color)
{
	this->pInternalContext->Clear(color);
}

ComPtr<ID2D1SolidColorBrush> RenderContext::createSolidColorBrush(const D2D1_COLOR_F& color)
{
	ComPtr<ID2D1SolidColorBrush> pbrTemp;
	
	ComResult(this->pInternalContext->CreateSolidColorBrush(color, &pbrTemp));
	return pbrTemp;
}
ComPtr<ID2D1LinearGradientBrush> RenderContext::createLinearGradientBrush(const D2D1_COLOR_F& firstColor, const D2D1_COLOR_F& secondColor)
{
	D2D1_GRADIENT_STOP stops[2];
	ComPtr<ID2D1GradientStopCollection> pCollection;
	ComPtr<ID2D1LinearGradientBrush> pbrTemp;
	ComResult hr;

	stops[0].color = firstColor;
	stops[0].position = 0.0f;
	stops[1].color = secondColor;
	stops[1].position = 1.0f;
	hr = this->pInternalContext->CreateGradientStopCollection(stops, 2, &pCollection);
	hr = this->pInternalContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(), D2D1::Point2F(0.0f, 1.0f)),
		pCollection.Get(), &pbrTemp);
	return pbrTemp;
}

void RenderContext::drawString(const std::wstring& str, const D2D1_POINT_2F& pt, IDWriteTextFormat* pFormat, ID2D1Brush* pBrush)
{
	this->pInternalContext->DrawTextW(str.c_str(), static_cast<UINT>(str.length()), pFormat, 
		D2D1::RectF(pt.x, pt.y, floatMax, floatMax), pBrush);
}
void RenderContext::drawString(const std::wstring& str, const D2D1_POINT_2F& pt, const std::wstring& formatKey, ID2D1Brush* pBrush)
{
	this->drawString(str, pt, getCurrentContext().getRenderDevice()->getStockedFormat(formatKey), pBrush);
}
void RenderContext::drawStringCenter(const std::wstring& str, const D2D1_RECT_F& rect, const std::wstring& formatKey, ID2D1Brush* pBrush)
{
	this->drawStringCenter(str, rect, getCurrentContext().getRenderDevice()->getStockedFormat(formatKey), pBrush);
}
void RenderContext::drawStringCenter(const std::wstring& str, const D2D1_RECT_F& rect, IDWriteTextFormat* pFormat, ID2D1Brush* pBrush)
{
	auto rw = rect.right - rect.left, rh = rect.bottom - rect.top;

	auto pSize = getCurrentContext().getRenderDevice()->calcStringSize(str, pFormat);
	auto pt = D2D1::Point2F(rect.left + (rw - pSize.width) / 2.0f, rect.top + (rh - pSize.height) / 2.0f);
	this->pInternalContext->DrawTextW(str.c_str(), static_cast<UINT>(str.length()), pFormat,
		D2D1::RectF(pt.x, pt.y, pt.x + pSize.width, pt.y + pSize.height), pBrush);
}
void RenderContext::drawStringHCenter(const std::wstring& str, float left, float right, float top, const std::wstring& formatKey, ID2D1Brush* pBrush)
{
	auto rw = right - left;
	auto pFormat = getCurrentContext().getRenderDevice()->getStockedFormat(formatKey);

	auto pSize = getCurrentContext().getRenderDevice()->calcStringSize(str, pFormat);
	auto pt = D2D1::Point2F(left + (rw - pSize.width) / 2.0f, top);
	this->pInternalContext->DrawTextW(str.c_str(), static_cast<UINT>(str.length()), pFormat,
		D2D1::RectF(pt.x, top, pt.x + pSize.width, top + pSize.height), pBrush);
}

void RenderContext::drawRectFrame(const D2D1_RECT_F& rect, ID2D1Brush* pBrush)
{
	this->pInternalContext->DrawRectangle(rect, pBrush);
}
void RenderContext::drawRect(const D2D1_RECT_F& rect, ID2D1Brush* pBrush)
{
	this->pInternalContext->FillRectangle(rect, pBrush);
}
void RenderContext::drawRoundedRect(const D2D1_RECT_F& rect, float radius, ID2D1Brush* pBrush)
{
	this->pInternalContext->FillRoundedRectangle(D2D1::RoundedRect(rect, radius, radius), pBrush);
}
void RenderContext::drawHorizontalLine(float y, float x1, float x2, ID2D1Brush* pBrush)
{
	this->pInternalContext->DrawLine(D2D1::Point2F(x1, y), D2D1::Point2F(x2, y), pBrush);
}
void RenderContext::drawVerticalLine(float x, float y1, float y2, ID2D1Brush* pBrush)
{
	this->pInternalContext->DrawLine(D2D1::Point2F(x, y1), D2D1::Point2F(x, y2), pBrush);
}