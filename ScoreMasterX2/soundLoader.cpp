#include "soundLoader.h"

#include "appContext.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <wrl.h>
#include "comutils.h"

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")

using namespace std::string_literals;

const auto InternalWaveNumChannels = 2;			// 2 channels(Left + Right)
const auto InternalWaveSamplesPerSec = 44100;	// 44100 Heltz
const auto InternalWaveBitDepth = 32;			// 32 bits, float
const auto InternalWaveBlockAlignment = InternalWaveNumChannels * InternalWaveBitDepth / 8;
const auto InternalWaveAvgBytesPerSec = InternalWaveSamplesPerSec * InternalWaveBlockAlignment;

SoundLoader::SoundLoader()
{
	this->pOverlay = std::make_unique<LoadingScreen>();

	this->hasExited.store(false);
	this->pThread = std::thread(&SoundLoader::WorkerProc, this);
}
SoundLoader::~SoundLoader()
{
	this->exitThread();
}

void SoundLoader::WorkerProc()
{
	OutputDebugString(L"Start SoundLoader thread.\n");
	MFStartup(MF_VERSION);
	while (true)
	{
		if (this->hasExited.load()) break;

		std::wstring filePathRetrieved;
		if (this->loaderQueue.try_pop(filePathRetrieved))
		{
			// success
			this->pOverlay->raiseEffect();

			// From Project::LoadSound //
			ComResult hr;
			ComPtr<IMFSourceResolver> pSourceResolver;
			ComPtr<IUnknown> pMediaSourceUnk;
			ComPtr<IMFMediaSource> pMediaSource;
			ComPtr<IMFSourceReader> pSourceReader;
			PROPVARIANT durationAttribute;
			MFTIME sourceDuration;
			ComPtr<IMFMediaType> pMediaType;

			OutputDebugString((L"Loading "s + filePathRetrieved + L"...\n"s).c_str());

			try
			{
				// Init Media Type Required converting to //
				hr = MFCreateMediaType(&pMediaType);
				hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
				hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
				hr = pMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, InternalWaveNumChannels);
				hr = pMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, InternalWaveSamplesPerSec);
				hr = pMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, InternalWaveBitDepth);
				hr = pMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, InternalWaveBlockAlignment);
				hr = pMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, InternalWaveAvgBytesPerSec);
				hr = pMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);

				// Load Media using SourceResolver&MediaSource //
				hr = MFCreateSourceResolver(&pSourceResolver);
				MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
				hr = pSourceResolver->CreateObjectFromURL(filePathRetrieved.c_str(), MF_RESOLUTION_MEDIASOURCE, nullptr, &objectType, &pMediaSourceUnk);
				hr = pMediaSourceUnk.As(&pMediaSource);
				hr = MFCreateSourceReaderFromMediaSource(pMediaSource.Get(), nullptr, &pSourceReader);
				hr = pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMediaType.Get());
				PropVariantInit(&durationAttribute);
				hr = pSourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &durationAttribute);
				sourceDuration = static_cast<MFTIME>(durationAttribute.uhVal.QuadPart);
				PropVariantClear(&durationAttribute);
				OutputDebugString((L"  Source Length: "s + std::to_wstring(sourceDuration / (10.0 * 1000.0 * 1000.0)) + L" secs\n"s).c_str());

				this->pOverlay->initLoadingProgress(filePathRetrieved, sourceDuration);

				std::array<std::vector<float>, 2> samples;
				while (true)
				{
					ComPtr<IMFSample> pSample;
					ComPtr<IMFMediaBuffer> pBuffer;
					DWORD actualStreamIndex, streamFlags, sampleBytes, bufferCount, bufferBytes;
					LONGLONG timestamp, sampleDuration;

					hr = pSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &actualStreamIndex, &streamFlags, &timestamp, &pSample);
					if ((streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) break;
					hr = pSample->GetTotalLength(&sampleBytes);
					hr = pSample->GetSampleDuration(&sampleDuration);
					hr = pSample->GetBufferCount(&bufferCount);
					hr = pSample->ConvertToContiguousBuffer(&pBuffer);
					hr = pBuffer->GetCurrentLength(&bufferBytes);
					// OutputDebugString((L"  Loading progress: "s + std::to_wstring((static_cast<double>(timestamp) / static_cast<double>(sourceDuration)) * 100.0) + L"%\n"s).c_str());
					/*OutputDebugString(L"  -- ReadSample Descriptions --\n");
					OutputDebugString((L"   ActualStreamIndex: "s + std::to_wstring(actualStreamIndex) + L"\n"s).c_str());
					OutputDebugString((L"   StreamFlags: "s + std::to_wstring(streamFlags) + L"\n"s).c_str());
					OutputDebugString((L"   Timestamp: "s + std::to_wstring(timestamp / (10.0 * 1000.0)) + L" ms\n"s).c_str());
					OutputDebugString((L"   Sample Bytes: "s + std::to_wstring(sampleBytes) + L"\n"s).c_str());
					OutputDebugString((L"   Sample Duration: "s + std::to_wstring(sampleDuration / (10.0 * 1000.0)) + L" ms\n"s).c_str());
					OutputDebugString((L"   Buffer Count: "s + std::to_wstring(bufferCount) + L"\n"s).c_str());
					OutputDebugString((L"   Buffer Length: "s + std::to_wstring(bufferBytes) + L"\n"s).c_str());*/

					float* ppBuffer;
					hr = pBuffer->Lock((BYTE**)&ppBuffer, nullptr, nullptr);
					for (int i = 0; i < bufferBytes / sizeof(float); i += 2)
					{
						samples[0].push_back(ppBuffer[i + 0]);
						samples[1].push_back(ppBuffer[i + 1]);
					}
					hr = pBuffer->Unlock();

					this->pOverlay->setCurrentProgress(timestamp);
				}
				
				getCurrentContext().getProjectManager()->getCurrent()->addWaveEntity(filePathRetrieved, samples);
				// OutputDebugString((L"Assertion: samples[0].length == samples[1].length ? "s + (samples[0].size() == samples[1].size() ? L"true"s : L"false"s) + L"\n"s).c_str());
			}
			catch (const _com_error& e)
			{
				OutputDebugString(L"Unsupported Format or Internal Error.\n");
			}
		}
		if (this->loaderQueue.empty()) this->pOverlay->falloffEffect();
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	MFShutdown();
	OutputDebugString(L"Exit SoundLoader thread.\n");
	this->varWaitTerminated.notify_one();
}

void SoundLoader::exitThread()
{
	this->hasExited.store(true);
	std::unique_lock<std::mutex> lock(this->terminatorMutex);
	while (true)
	{
		auto st = this->varWaitTerminated.wait_for(lock, std::chrono::milliseconds(1));
		if (st == std::cv_status::no_timeout) break;
		getCurrentContext().processMessagesForWaitFrame();
	}
	this->pThread.join();
}
void SoundLoader::sendToLoad(const std::wstring& filePath)
{
	this->loaderQueue.push(filePath);
}

LoadingScreen::LoadingScreen() : Layer(D2D1::SizeF(100, 100))
{
	this->effectRaised = false;

	this->pAlphaAnimator = std::make_unique<LayerAnimation>();
	this->pOpacity = std::make_unique<LayerOpacityEffect>();

	this->getVisual()->SetEffect(this->pOpacity->getEffectObject());
	this->pOpacity->setOpacity(0.0f);
}
LoadingScreen::~LoadingScreen() = default;
void LoadingScreen::updateContent(RenderContext* pRenderContext)
{
	auto pLoadingTextBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black));
	auto pProgressBackBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.1875f));
	auto pProgressForeBrush = pRenderContext->createSolidColorBrush(D2D1::ColorF(0x66ccff, 0.75f));

	pRenderContext->clear(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
	if (this->timeTotal >= 0)
	{
		const auto progressRect = D2D1::RectF(this->getSize().width * 0.125f, this->getSize().height * 0.5f + 16.0f, this->getSize().width * (1.0f - 0.125f), this->getSize().height * 0.5f + 16.0f + 4.0f);
		const auto progressWidth = progressRect.right - progressRect.left;
		const auto progressPercent = static_cast<float>(this->timeCurrent) / static_cast<float>(this->timeTotal);

		pRenderContext->drawStringCenter(L"Loading: "s + this->filePath, this->getLocalRect(), L"uiDefault", pLoadingTextBrush.Get());
		pRenderContext->drawRoundedRect(progressRect, 2.0f, pProgressBackBrush.Get());
		pRenderContext->drawRoundedRect(D2D1::RectF(progressRect.left, progressRect.top, progressRect.left + progressWidth * progressPercent, progressRect.bottom),
			2.0f, pProgressForeBrush.Get());
	}
}

void LoadingScreen::raiseEffect()
{
	if (!this->effectRaised)
	{
		this->timeTotal = -1;
		getCurrentContext().queueUpdated(this);
		this->pAlphaAnimator->setInverseQuadratic(0.25, 0.0f, 1.0f);
		this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
		this->pAlphaAnimator->commit();
	}
	this->effectRaised = true;
}
void LoadingScreen::falloffEffect()
{
	if (this->effectRaised)
	{
		this->pAlphaAnimator->setLinear(0.25, 1.0f, 0.0f);
		this->pOpacity->animateOpacity(this->pAlphaAnimator.get());
		this->pAlphaAnimator->commit();
	}
	this->effectRaised = false;
}

void LoadingScreen::initLoadingProgress(const std::wstring& filePath, const MFTIME& timeTotal)
{
	this->timeTotal = timeTotal;
	this->filePath = filePath;
	this->timeCurrent = 0;
	getCurrentContext().queueUpdated(this);
}
void LoadingScreen::setCurrentProgress(const MFTIME& timeCurrent)
{
	this->timeCurrent = timeCurrent;
	getCurrentContext().queueUpdated(this);
}