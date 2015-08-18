#pragma once

#include "layerManager.h"
#include "project.h"
#include <iterator>
#include "arrangerToolComponents.h"
#include "scrollBars.h"

const auto MeasureHeight = 20.0f;
const auto TracklistWidth = 150.0f;
const auto TrackHeight = 24.0f;
const auto SplitterHeight = 4.0f;
const auto BeatWidth = 48.0f;

class ArrangerSpaceMask final : public Layer
{
public:
	ArrangerSpaceMask();
	~ArrangerSpaceMask();
protected:
	void updateContent(RenderContext* pRenderContext) override;
};
class ArrangerBorder final : public Layer
{
public:
	ArrangerBorder();
	~ArrangerBorder();
protected:
	void updateContent(RenderContext* pRenderContext) override;
};

class ArrangerTools final : public Layer
{
	class AddTrackCommandButton : public ToolCommandButton
	{
	public:
		AddTrackCommandButton();
		~AddTrackCommandButton();

		void onClick() override;
	};
	class RemoveTrackCommandButton : public ToolCommandButton
	{
	public:
		RemoveTrackCommandButton();
		~RemoveTrackCommandButton();

		void onClick() override;
	};
	class QuantizeIndicator final : public Layer
	{
		class QuantizeDrop final : public Layer
		{
			class BorderOverlay final : public Layer
			{
				std::unique_ptr<LayerAnimation> pAlphaAnimator;
				std::unique_ptr<LayerOpacityEffect> pOpacity;
			public:
				BorderOverlay();
				~BorderOverlay();

				void raiseEffect();
				void falloffEffect();
			protected:
				void updateContent(RenderContext* pRenderContext) override;
			};
			class ArrowOverlay final : public Layer
			{
				std::unique_ptr<LayerAnimation> pAlphaAnimator, pVerticalAnimator;
				std::unique_ptr<LayerOpacityEffect> pOpacity;
			public:
				ArrowOverlay();
				~ArrowOverlay();

				void raiseEffect();
				void falloffEffect();
			protected:
				void updateContent(RenderContext* pRenderContext) override;
			};

			std::unique_ptr<BorderOverlay> pBorderOverlay;
			std::unique_ptr<ArrowOverlay> pArrowOverlay;
		public:
			QuantizeDrop();
			~QuantizeDrop();

			void updateAll() override;
			void onMouseEnter() override;
			void onMouseLeave() override;
			void onMouseDown() override;
		protected:
			void updateContent(RenderContext* pRenderContext) override;
		};

		std::unique_ptr<QuantizeDrop> pQuantizeDrop;
	public:
		QuantizeIndicator();
		~QuantizeIndicator();
		void notifyUpdateQuantizeValue();

		void updateAll() override;
		void onMouseMove(const D2D1_POINT_2F& pt) override;
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	std::unique_ptr<AddTrackCommandButton> pAddTrackCommandButton;
	std::unique_ptr<RemoveTrackCommandButton> pRemoveTrackCommandButton;
	std::unique_ptr<QuantizeIndicator> pQuantizeIndicator;
public:
	ArrangerTools();
	~ArrangerTools();

	void adjustWidth(float w);
	void notifyUpdateQuantizeValue() { this->pQuantizeIndicator->notifyUpdateQuantizeValue(); }

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
};

class MeasureBar final : public Layer
{
public:
	MeasureBar();
	~MeasureBar();

	void adjustWidth(float w);
protected:
	void updateContent(RenderContext* pRenderContext) override;
};

template<typename ContainerT>
class ArrangerInnerViewBase : public Layer
{
public:
	using IteratorT = typename ContainerT::const_iterator;
private:
	class TrackView : public Layer
	{
		ArrangerInnerViewBase<ContainerT>* pOuterView;
	public:
		TrackView(ArrangerInnerViewBase<ContainerT>* pout) : Layer()
		{
			this->pOuterView = pout;
		}
		~TrackView() = default;

		void adjustHeight(float h) { this->resize(D2D1::SizeF(TracklistWidth, h)); }
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};
	class WaveMatrix : public Layer
	{
		ArrangerInnerViewBase<ContainerT>* pOuterView;
	public:
		WaveMatrix(ArrangerInnerViewBase<ContainerT>* pout)
		{
			this->pOuterView = pout;
		}
		~WaveMatrix() = default;
	protected:
		void updateContent(RenderContext* pRenderContext) override;
	};

	ContainerT iter_dummy;
	IteratorT track_begin, track_end;
	std::unique_ptr<TrackView> pTrackView;
	std::unique_ptr<WaveMatrix> pWaveMatrix;
public:
	ArrangerInnerViewBase() : Layer()
	{
		this->track_begin = this->track_end = std::cend(iter_dummy);

		this->pTrackView = std::make_unique<TrackView>(this);
		this->pWaveMatrix = std::make_unique<WaveMatrix>(this);

		this->addChild(this->pTrackView.get());
		this->addChild(this->pWaveMatrix.get());
		this->pWaveMatrix->setOffset(D2D1::Point2F(TracklistWidth, 0.0f));
	}
	virtual ~ArrangerInnerViewBase() = default;

	auto getTrackLayer() { return this->pTrackView.get(); }
	auto getMatrixLayer() { return this->pWaveMatrix.get(); }
	auto getInnerHeight() { return std::distance(this->track_begin, this->track_end) * TrackHeight; }
	void updateTrackRange(IteratorT begin_iter, IteratorT end_iter);

	virtual void exRenderForWaveMatrix(RenderContext* pRenderContext, uint32_t trackIndex, const D2D1_RECT_F& renderArea) {}

	void updateAll() override
	{
		this->Layer::updateAll();

		this->pTrackView->updateAll();
		this->pWaveMatrix->updateAll();
	}
protected:
	void resizeContent(const D2D1_SIZE_F& size) override
	{
		this->pTrackView->adjustHeight(size.height);
		this->pWaveMatrix->resize(D2D1::SizeF(size.width - TracklistWidth, size.height));
	}
};

class SystemTrackView final : public ArrangerInnerViewBase<Project::SystemTrackListT>
{
	// Special Render for WaveMatrix
public:
	SystemTrackView();
	~SystemTrackView();

	void exRenderForWaveMatrix(RenderContext* pRenderContext, uint32_t trackIndex, const D2D1_RECT_F& renderArea) override;
};
using UserTrackView = ArrangerInnerViewBase<Project::UserTrackListT>;

class ArrangerView final : public Layer, public IScrollBarValueReceptor
{
	std::unique_ptr<ArrangerBorder> pArrangerBorder;
	std::unique_ptr<ArrangerSpaceMask> pSpaceMask, pSpaceMaskUnder;
	std::unique_ptr<ArrangerTools> pArrangerTools;
	std::unique_ptr<MeasureBar> pMeasureBar;
	std::unique_ptr<SystemTrackView> pSysTrackView;
	std::unique_ptr<UserTrackView> pUserTrackView;
	std::unique_ptr<HorizontalScrollBar> pMeasScrollBar;
	std::unique_ptr<VerticalScrollBar> pTrackScrollBar;
public:
	ArrangerView();
	~ArrangerView();

	void updateSystemTracks(SystemTrackView::IteratorT begin_iter, SystemTrackView::IteratorT end_iter);
	void updateUserTracks(UserTrackView::IteratorT begin_iter, UserTrackView::IteratorT end_iter);

	auto getTrackScrollOffset() { return this->pTrackScrollBar->getCurrentValue(); }
	auto getMeasureScrollOffset() { return this->pMeasScrollBar->getCurrentValue(); }
	auto notifyUpdateQuantizeValue() { this->pArrangerTools->notifyUpdateQuantizeValue(); }

	void updateAll() override;
	void onMouseMove(const D2D1_POINT_2F& pt) override;
	void receiveValueChanged(ScrollBarBase* pSender) override;
protected:
	void resizeContent(const D2D1_SIZE_F& size) override;
};

// TEMPLATE IMPLEMENTATIONS //
#include "appContext.h"

template<typename ContainerT>
void ArrangerInnerViewBase<ContainerT>::updateTrackRange(IteratorT begin_iter, IteratorT end_iter)
{
	this->track_begin = begin_iter;
	this->track_end = end_iter;
	getCurrentContext().queueUpdated(this->pTrackView.get());
	getCurrentContext().queueUpdated(this->pWaveMatrix.get());
}
template<typename ContainerT>
void ArrangerInnerViewBase<ContainerT>::TrackView::updateContent(RenderContext* pRenderContext)
{
	auto pLineBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.25f));

	pRenderContext->clear(D2D1::ColorF(0x404040));
	pRenderContext->drawVerticalLine(this->getSize().width - 0.5f, 0.0f, this->getSize().height, pLineBrush.Get());

	if (this->pOuterView->track_begin != this->pOuterView->track_end)
	{
		auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White));
		auto pInvalidMaskBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.25f));
		auto pTrackBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(0));

		// render
		auto y = 0.5f;
		auto ryoffs = static_cast<float>(getCurrentContext().getArrangerView()->getTrackScrollOffset());
		for (auto iter = this->pOuterView->track_begin; iter != this->pOuterView->track_end; ++iter)
		{
			auto render_y = y - ryoffs;
			if (render_y >= this->getSize().height) break;
			if (render_y >= -TrackHeight)
			{
				pTrackBrush->SetColor((*iter)->getTrackColor());

				pRenderContext->drawString((*iter)->getName(), D2D1::Point2F(6.0f, render_y + 4.0f), L"uiDefault", pTextBrush.Get());
				auto trackActivatorRect = D2D1::RectF(this->getSize().width - 3.0f - 3.0f, render_y + 3.0f + 0.5f,
					this->getSize().width - 3.0f, render_y + TrackHeight - 3.0f - 0.5f);
				pRenderContext->drawRect(trackActivatorRect, pTrackBrush.Get());
				if ((*iter)->isActivated() == false) pRenderContext->drawRect(trackActivatorRect, pInvalidMaskBrush.Get());
				if ((*iter)->isEditable() == false)
				{
					pRenderContext->drawRect(D2D1::RectF(0.0f, render_y, this->getSize().width - 0.5f, render_y + TrackHeight - 0.5f),
						pInvalidMaskBrush.Get());
				}
				pRenderContext->drawHorizontalLine(render_y + TrackHeight, 0.0f, this->getSize().width - 0.5f, pLineBrush.Get());
			}
			y += TrackHeight;
		}
	}
}

template<typename ContainerT>
void ArrangerInnerViewBase<ContainerT>::WaveMatrix::updateContent(RenderContext* pRenderContext)
{
	auto pLineBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.25f));
	auto pBarBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5f));

	pRenderContext->clear(D2D1::ColorF(0x202020));

	auto ch = this->getSize().height;
	auto beatOffset = static_cast<uint32_t>(floor(getCurrentContext().getArrangerView()->getMeasureScrollOffset() / BeatWidth));
	auto x = -fmodf(static_cast<float>(getCurrentContext().getArrangerView()->getMeasureScrollOffset()), BeatWidth) + 0.5f;
	for (uint32_t beat = beatOffset; ; beat++)
	{
		if (x > this->getSize().width) break;
		pRenderContext->drawVerticalLine(x, 0.0f, ch, beat % 4 == 0 ? pBarBrush.Get() : pLineBrush.Get());
		x += BeatWidth;
	}

	if (this->pOuterView->track_begin != this->pOuterView->track_end)
	{
		auto pTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White));
		auto pInvalidMaskBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.25f));

		// render
		auto y = 0.5f;
		auto ryoffs = static_cast<float>(getCurrentContext().getArrangerView()->getTrackScrollOffset());
		auto rxoffs = static_cast<float>(getCurrentContext().getArrangerView()->getMeasureScrollOffset());
		auto renderIndex = 0;
		for (auto iter = this->pOuterView->track_begin; iter != this->pOuterView->track_end; ++iter)
		{
			auto render_y = y - ryoffs;
			if (render_y >= this->getSize().height) break;
			if (render_y >= -TrackHeight)
			{
				if ((*iter)->isEditable() == false)
				{
					pRenderContext->drawRect(D2D1::RectF(0.0f, render_y, this->getSize().width - 0.5f, render_y + TrackHeight - 0.5f),
						pInvalidMaskBrush.Get());
				}
				this->pOuterView->exRenderForWaveMatrix(pRenderContext, renderIndex, D2D1::RectF(-rxoffs, render_y, this->getSize().width - 0.5f, render_y + TrackHeight));
				pRenderContext->drawHorizontalLine(render_y + TrackHeight, 0.0f, this->getSize().width - 0.5f, pLineBrush.Get());
			}
			y += TrackHeight;
			renderIndex++;
		}
	}
}