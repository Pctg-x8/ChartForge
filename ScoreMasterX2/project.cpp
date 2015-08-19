#include "project.h"

#include <windows.h>
#include <random>
#include <cmath>
#include "appContext.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <wrl.h>
#include "comutils.h"
#include "wavePaletteView.h"

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")

using Microsoft::WRL::ComPtr;

using namespace std::string_literals;

D2D1_COLOR_F makeRandomTrackColor()
{
	static std::random_device rd;
	static std::mt19937 randomizer(rd());
	std::uniform_real_distribution<> range(0, 360);

	auto hue = range(randomizer);
	auto sat = 255.0;
	auto val = 255.0f;

	auto hi = static_cast<uint32_t>(floor(hue / 60.0)) % 6;
	auto f = static_cast<float>((hue / 60.0) - floor(hue / 60.0));
	auto p = static_cast<float>(round(val * (1.0 - (sat / 255.0))));
	auto q = static_cast<float>(round(val * (1.0 - (sat / 255.0) * f)));
	auto t = static_cast<float>(round(val * (1.0 - (sat / 255.0) * (1.0 - f))));

	switch (hi)
	{
	case 0: return D2D1::ColorF(val / 255.0f, t / 255.0f, p / 255.0f);
	case 1: return D2D1::ColorF(q / 255.0f, val / 255.0f, p / 255.0f);
	case 2: return D2D1::ColorF(p / 255.0f, val / 255.0f, t / 255.0f);
	case 3: return D2D1::ColorF(p / 255.0f, q / 255.0f, val / 255.0f);
	case 4: return D2D1::ColorF(t / 255.0f, p / 255.0f, val / 255.0f);
	case 5: return D2D1::ColorF(val / 255.0f, p / 255.0f, q / 255.0f);
	default: return D2D1::ColorF(D2D1::ColorF::White);
	}
}

Track::Track(const std::wstring& n, bool ie, bool ia, const D2D1_COLOR_F& col) : name(n)
{
	this->is_editable = ie;
	this->is_activated = ia;
	this->trackColor = col;
}
Track::~Track() = default;

Project::Project()
{
	this->qtzDenom = 16.0;

	this->tempoInfos.emplace_back(SongPosition(0, 0, 0), 120.0f);
	this->beatInfos.emplace_back(0, 4, 4);
	this->systemTracks[0] = std::make_shared<Track>(L"Tempo Track", false, false, D2D1::ColorF(D2D1::ColorF::White));
	this->systemTracks[1] = std::make_shared<Track>(L"Beat Track", false, false, D2D1::ColorF(D2D1::ColorF::White));
	this->systemTracks[2] = std::make_shared<Track>(L"Stop Sequences", false, false, D2D1::ColorF(D2D1::ColorF::White));
	this->userTracks.clear();
}
Project::~Project() = default;

void Project::addEmptyTrack()
{
	this->userTracks.emplace_back(std::make_shared<Track>(L"Track "s + std::to_wstring(this->userTracks.size() + 1), true, true, makeRandomTrackColor()));
}
void Project::addWaveEntity(const std::wstring& filePath, const std::array<std::vector<float>, 2>& pDataBase)
{
	this->waveEntityList.push_back(std::make_unique<WaveEntity>(filePath, pDataBase));
	getCurrentContext().queueUpdated(getCurrentContext().getWavePaletteView()->getWaveListView()->getInnerView());
}

ProjectManager::ProjectManager()
{
	this->createEmpty();
}
ProjectManager::~ProjectManager() = default;
void ProjectManager::createEmpty()
{
	wchar_t pwd[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, pwd);
	this->filePath = pwd;
	this->filePath.append(L"\\untitled.cfp");
	this->pProjectInstance = std::make_unique<Project>();
}

WaveEntity::WaveEntity(const std::wstring& filePath, const std::array<std::vector<float>, 2>& pDataBase)
{
	this->filePath = filePath;
	this->dataLength = pDataBase[0].size();
	this->pDataEntity[0].reset(new float[this->dataLength]);
	this->pDataEntity[1].reset(new float[this->dataLength]);

	std::memcpy(this->pDataEntity[0].get(), pDataBase[0].data(), pDataBase[0].size() * sizeof(float));
	std::memcpy(this->pDataEntity[1].get(), pDataBase[1].data(), pDataBase[1].size() * sizeof(float));
}