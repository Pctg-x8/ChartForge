#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <dxgi.h>
#include <dwrite.h>
#include <wrl.h>

#include <set>
#include <unordered_map>

using Microsoft::WRL::ComPtr;

class RenderDevice
{
	ComPtr<ID3D11Device> pDevice3;
	ComPtr<ID3D11DeviceContext> pDeviceContext3;
	ComPtr<IDXGIDevice1> pxDevice;
	ComPtr<ID2D1Device> pDevice2;
	ComPtr<IDWriteFactory> pdwFactory;
	ComPtr<IDWriteFontCollection> pSystemFontCollection;

	std::set<std::wstring> dwfstock_keypool;
	std::unordered_map<const std::wstring*, std::pair<ComPtr<IDWriteTextFormat>, ComPtr<IDWriteFont>>> dwFormatStock;
public:
	RenderDevice();
	~RenderDevice();

	void init();

	auto getDevice2() { return this->pDevice2.Get(); }
	void stockFormat(const std::wstring& key, const std::wstring& familyName, float size,
		DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL);
	IDWriteTextFormat* getStockedFormat(const std::wstring& key);
	IDWriteFont* getStockedFont(const std::wstring& key);
	float calcStringWidth(const std::wstring& strMeasured, const std::wstring& key);
	D2D1_SIZE_F calcStringSize(const std::wstring& strMeasured, IDWriteTextFormat* pFormat);
	float calcBaselineAscent(const std::wstring& key);
};

class RenderContext
{
	// Direct2D Rendering Context Wrapper
	ComPtr<ID2D1DeviceContext> pInternalContext;
public:
	RenderContext(const ComPtr<ID2D1DeviceContext>& pContextBase);
	~RenderContext();

	void clearAsAppBackground();
	void clearAsComponentBackground();
	void clear(const D2D1_COLOR_F& color);

	ComPtr<ID2D1SolidColorBrush> createSolidColorBrush(const D2D1_COLOR_F& color);
	ComPtr<ID2D1LinearGradientBrush> createLinearGradientBrush(const D2D1_COLOR_F& firstColor, const D2D1_COLOR_F& secondColor);

	void drawString(const std::wstring& str, const D2D1_POINT_2F& pt, IDWriteTextFormat* pFormat, ID2D1Brush* pBrush);
	void drawString(const std::wstring& str, const D2D1_POINT_2F& pt, const std::wstring& formatKey, ID2D1Brush* pBrush);
	void drawStringCenter(const std::wstring& str, const D2D1_RECT_F& rect, const std::wstring& formatKey, ID2D1Brush* pBrush);
	void drawStringCenter(const std::wstring& str, const D2D1_RECT_F& rect, IDWriteTextFormat* pFormat, ID2D1Brush* pBrush);
	void drawStringHCenter(const std::wstring& str, float left, float right, float top, const std::wstring& formatKey, ID2D1Brush* pBrush);

	void drawRectFrame(const D2D1_RECT_F& rect, ID2D1Brush* pBrush);
	void drawRect(const D2D1_RECT_F& rect, ID2D1Brush* pBrush);
	void drawRoundedRect(const D2D1_RECT_F& rect, float radius, ID2D1Brush* pBrush);
	void drawHorizontalLine(float y, float x1, float x2, ID2D1Brush* pBrush);
	void drawVerticalLine(float x, float y1, float y2, ID2D1Brush* pBrush);
};
