#pragma once

#include "layerManager.h"
#include <thread>
#include <atomic>
#include <string>
#include <concurrent_queue.h>
#include <mutex>
#include <condition_variable>
#include <mfidl.h>

class LoadingScreen final : public Layer
{
	bool effectRaised;

	std::unique_ptr<LayerAnimation> pAlphaAnimator;
	std::unique_ptr<LayerOpacityEffect> pOpacity;

	// For Progress reflection //
	MFTIME timeTotal, timeCurrent;
	std::wstring filePath;
public:
	LoadingScreen();
	~LoadingScreen();

	auto isEffectRaised() { return this->effectRaised; }

	void initLoadingProgress(const std::wstring& filePath, const MFTIME& timeTotal);
	void setCurrentProgress(const MFTIME& timeCurrent);

	void raiseEffect();
	void falloffEffect();
protected:
	void updateContent(RenderContext* pRenderContext) override;
};

class SoundLoader
{
	std::unique_ptr<LoadingScreen> pOverlay;

	std::thread pThread;
	std::atomic<bool> hasExited;
	concurrency::concurrent_queue<std::wstring> loaderQueue;
	std::mutex terminatorMutex;
	std::condition_variable varWaitTerminated;

	void WorkerProc();
public:
	SoundLoader();
	~SoundLoader();

	void exitThread();
	void sendToLoad(const std::wstring& filePath);

	auto getOverlay() { return this->pOverlay.get(); }
};