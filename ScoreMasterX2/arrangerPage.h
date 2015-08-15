#pragma once

#include "layerManager.h"

class ArrangerView;
class WavePaletteView;
class ArrangerPage final : public Layer
{
	class Splitter final : public Layer
	{
		class Overlay final : public Layer
		{
			std::unique_ptr<LayerAnimation> pAlphaAnimator;
			std::unique_ptr<LayerOpacityEffect> pOpacity;
		public:
			Overlay();
			~Overlay();

			void raiseEffect();
			void falloffEffect();
		protected:
			void updateContent(RenderContext* pRenderContext) override;
		};

		ArrangerPage* pParentObject;
		D2D1_POINT_2F lastOffset;

		std::unique_ptr<Overlay> pClickableOverlay, pPressingOverlay;
	public:
		Splitter(ArrangerPage* pParent);
		~Splitter();

		void adjustHeight(float h);

		void updateAll() override;
		void onMouseMove(const D2D1_POINT_2F& pt) override;
		void onMouseEnter() override;
		void onMouseLeave() override;
		void onMouseDown() override;
		void onMouseUp() override;
	};

	float layoutOffset, layoutRight;

	std::unique_ptr<ArrangerView> pArrangerView;
	std::unique_ptr<Splitter> pSplitter;
	std::unique_ptr<WavePaletteView> pWavePaletteView;
public:
	ArrangerPage();
	~ArrangerPage();

	auto getLeftView() { return this->pArrangerView.get(); }
	auto getRightView() { return this->pWavePaletteView.get(); }

	void startLayouting(const D2D1_POINT_2F& pt);
	void moveSplitter(const D2D1_POINT_2F& pt);

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};