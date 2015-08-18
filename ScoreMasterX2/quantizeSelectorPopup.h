#pragma once

#include "PopupFrame.h"
#include <array>
#include <initializer_list>

struct UnmanagedLayer
{
	ComPtr<IDCompositionVisual2> pVisual;
	ComPtr<IDCompositionSurface> pSurface;

	UnmanagedLayer(float initWidth = 100.0f, float initHeight = 100.0f);
};

template<uint32_t Num, uint32_t Denom>
struct Radial
{
	static const auto Numerator = Num;
	static const auto Denominator = Denom;
	static constexpr auto Value = static_cast<double>(Num) / static_cast<double>(Denom);
};
template<typename Rad>
double RadialCons() { return Rad::Value; }

template<typename... Values>
struct QuantizeElementsT
{
	static const auto Size = sizeof...(Values);
	static auto getValueArray() { return std::array<double, Size>{ (RadialCons<Values>())... }; }
};

using QuantizeElements = QuantizeElementsT<
	Radial<1, 4>, Radial<1, 2>, Radial<1, 1>,
	Radial<2, 1>, Radial<3, 1>, Radial<4, 1>, Radial<6, 1>, Radial<8, 1>,
	Radial<12, 1>, Radial<16, 1>, Radial<24, 1>,
	Radial<32, 1>, Radial<48, 1>, Radial<64, 1>, Radial<96, 1>,
	Radial<128, 1>, Radial<192, 1>, Radial<256, 1>
>;
const auto QuantizeElementHeight = 20.0f;
const auto PopupHeight = QuantizeElementHeight * QuantizeElements::Size;

class QuantizeSelectorPopup final : public PopupFrame
{
	class SelectorElement final
	{
		std::unique_ptr<UnmanagedLayer> pLabelLayer, pIndicateLayer;
		std::unique_ptr<LayerAnimation> pAlphaAnimator, pHorizontalAnimator;
		std::unique_ptr<LayerOpacityEffect> pLabelOpacityEffect;
		std::unique_ptr<LayerAnimation> pIndicateAlphaAnimator;
		std::unique_ptr<LayerOpacityEffect> pIndicateLayerOpacity;

		bool hasIndicatedSelection;
	public:
		SelectorElement(double value);
		~SelectorElement() = default;

		auto getLabelLayer() { return this->pLabelLayer.get(); }
		auto getIndicateLayer() { return this->pIndicateLayer.get(); }
		
		void raiseEffect(double delays);
		void falloffEffect();
		void raiseIndicator();
		void falloffIndicator();
	};
	std::unique_ptr<UnmanagedLayer> pBackLayer;
	std::unique_ptr<LayerAnimation> pAlphaAnimator;
	std::unique_ptr<LayerOpacityEffect> pOpacityEffect;
	std::array<std::unique_ptr<SelectorElement>, QuantizeElements::Size> pSelectorElements;
	int32_t lastSelectIndex;

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	void updateSelectionIndicators(LPARAM);
public:
	QuantizeSelectorPopup();
	~QuantizeSelectorPopup();

	void onShow() override;
	void onHide() override;
};