#pragma once

#include <string>
#include <memory>
#include <array>
#include <list>
#include <deque>
#include <vector>
#include <d2d1.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")

D2D1_COLOR_F makeRandomTrackColor();

struct SongPosition
{
	uint32_t barCount, beatNum, beatDenom;

	SongPosition(uint32_t bc, uint32_t bn, uint32_t bd)
	{
		this->barCount = bc;
		this->beatNum = bn;
		this->beatDenom = bd;
	}
	SongPosition(const SongPosition& ref)
	{
		this->barCount = ref.barCount;
		this->beatNum = ref.beatNum;
		this->beatDenom = ref.beatDenom;
	}
	auto serializeBarValue() const
	{
		return this->barCount + (static_cast<double>(this->beatNum) / static_cast<double>(this->beatDenom));
	}
	auto getSmallOffset() const
	{
		if (this->beatDenom == 0) return 0.0;
		return static_cast<double>(this->beatNum) / static_cast<double>(this->beatDenom);
	}

	bool operator>(const SongPosition& t) const
	{
		return t.barCount < this->barCount || t.getSmallOffset() < this->getSmallOffset();
	}
	bool operator<(const SongPosition& t) const
	{
		return t.barCount > this->barCount || t.getSmallOffset() > this->getSmallOffset();
	}
	bool operator==(const SongPosition& t) const
	{
		return t.barCount == this->barCount && t.beatNum == this->beatNum && t.beatDenom == this->beatDenom;
	}
};

struct TempoInfo
{
	// tempo info
	SongPosition pos;
	double tempo;

	TempoInfo(const SongPosition& p, double t) : pos(p)
	{
		this->tempo = t;
	}
};
struct BeatInfo
{
	// beat info
	uint32_t barCount;
	uint32_t beatNum, beatDenom;

	BeatInfo(uint32_t barCount, uint32_t beatNum, uint32_t beatDenom)
	{
		this->barCount = barCount;
		this->beatNum = beatNum;
		this->beatDenom = beatDenom;
	}
};
class WaveEntity;
class WaveObject
{
	// wave object
	SongPosition pos;
	std::shared_ptr<WaveEntity> pWaveEntity;
public:
	WaveObject(const SongPosition& p, const std::shared_ptr<WaveEntity>& pw);
	~WaveObject();

	auto getPosition() const { return this->pos; }
	auto getWaveEntity() const { return this->pWaveEntity.get(); }
};

class WaveEntity
{
	// wave entity
	std::wstring filePath;
	size_t dataLength;
	double lengthInSeconds;
	std::array<std::unique_ptr<float[]>, 2> pDataEntity;
	std::vector<WaveObject*> usedWaveObject;
public:
	WaveEntity(const std::wstring& filePath, double lis, const std::array<std::vector<float>, 2>& pDataBase);

	auto getFilePath() const { return this->filePath; }
	auto getFileName()
	{
		auto pFileName = PathFindFileName(this->filePath.c_str());
		return std::wstring(pFileName);
	}
	auto getDataLength() const { return this->dataLength; }
	auto getSourceLengthInSeconds() const { return this->lengthInSeconds; }
	auto getLeftEntity() { return this->pDataEntity[0].get(); }
	auto getRightEntity() { return this->pDataEntity[1].get(); }
	void registerUsedObject(WaveObject* po) { this->usedWaveObject.push_back(po); }
	void unregisterUsedObject(WaveObject* po)
	{
		this->usedWaveObject.erase(std::remove_if(std::begin(this->usedWaveObject), std::end(this->usedWaveObject),
			[=](WaveObject* p) { return p == po; }), std::end(this->usedWaveObject));
	}
};

class Track final
{
	// Track Structure
	std::wstring name;
	bool is_editable;
	bool is_activated;
	D2D1_COLOR_F trackColor;
	std::deque<std::unique_ptr<WaveObject>> objectList;
public:
	Track(const std::wstring& n, bool ie, bool ia, const D2D1_COLOR_F& col);
	~Track();

	void insertObject(const SongPosition& p, const std::shared_ptr<WaveEntity>& pw);

	auto getName() const { return this->name; }
	auto isEditable() const { return this->is_editable; }
	auto isActivated() const { return this->is_activated; }
	auto getTrackColor() const { return this->trackColor; }
	auto hasObject() const { return !this->objectList.empty(); }
	auto isSoloObject() const { return this->objectList.size() == 1; }
	auto getObject(size_t i) const { return this->objectList.at(i).get(); }
	auto getFirstObjectIter() const { return std::begin(this->objectList); }
	auto getLastObjectIter() const { return std::end(this->objectList) - 1; }
	auto getEndObjectIter() const { return std::end(this->objectList); }
};

class Project
{
	// Project Structure
public:
	using SystemTrackListT = std::array<std::shared_ptr<Track>, 3>;
	using UserTrackListT = std::vector<std::shared_ptr<Track>>;
private:
	SystemTrackListT systemTracks;		// for visualization
	UserTrackListT userTracks;

	std::list<TempoInfo> tempoInfos;
	std::list<BeatInfo> beatInfos;
	std::vector<std::shared_ptr<WaveEntity>> waveEntityList;

	SongPosition current;
	double qtzDenom;
public:
	Project();
	~Project();

	void addEmptyTrack();
	void addWaveEntity(const std::wstring& filePath, double lis, const std::array<std::vector<float>, 2>& pDataBase);
	void commitUpdateWaveEntity();
	void insertObjectToTrack(size_t userTrackIndex, const SongPosition& pos, const std::shared_ptr<WaveEntity>& pWaveEntityRef);

	auto getBeginIterator_SystemTracks() const { return std::cbegin(this->systemTracks); }
	auto getEndIterator_SystemTracks() const { return std::cend(this->systemTracks); }
	auto getBeginIterator_UserTracks() const { return std::cbegin(this->userTracks); }
	auto getEndIterator_UserTracks() const { return std::cend(this->userTracks); }
	auto getUserTrackCount() const { return this->userTracks.size(); }
	auto getUserTrack(size_t i) const { return this->userTracks.at(i).get(); }
	auto getBeginIterator_WaveEntities() const { return std::cbegin(this->waveEntityList); }
	auto getIteratorAt_WaveEntities(size_t v) const { return std::cbegin(this->waveEntityList) + v; }
	auto getEndIterator_WaveEntities() const { return std::cend(this->waveEntityList); }
	auto getWaveEntity(size_t i) const { return this->waveEntityList.at(i); }
	auto getWaveEntityCount() const { return this->waveEntityList.size(); }
	const auto& getTempoList() const { return this->tempoInfos; }
	const auto& getBeatList() const { return this->beatInfos; }
	auto getQuantizeValue() const { return 1.0f / this->qtzDenom; }
	auto getQuantizeValueInv() const { return this->qtzDenom; }
	void setQuantizeValueInv(double v) { this->qtzDenom = v; }
	auto getCurrentPosition() const { return this->current; }
	void setCurrentPosition(const SongPosition& p);
	double getCurrentTempo() const;
};

class ProjectManager
{
	std::wstring filePath;
	std::unique_ptr<Project> pProjectInstance;
public:
	ProjectManager();
	~ProjectManager();

	void createEmpty();
	auto getFilePath() { return this->filePath; }
	auto getCurrent() { return this->pProjectInstance.get(); }
};